// Name: Sarthak Singhal Roll No.: 170635

#include "common.h"
/*Function templates. TODO*/

static int atomic_add(unsigned *ptr, long val)
{
       int ret = 0;
       asm volatile( 
                     "lock add %%rsi, (%%rdi);"
                     "pushf;"
                     "pop %%rax;" 
                     "movl %%eax, %0;"
                     : "=r" (ret)
                     : 
                     : "memory", "rax"
                    );

     
     if(ret & 0x80)
               ret = -1;
     else if(ret & 0x40)
               ret = 0;
     else
               ret = 1;
     return ret;
}

void done_one(struct input_manager *in, int tnum)
{
	pthread_mutex_lock(&(in->lock));
	in->being_processed[tnum-1] = NULL;
	pthread_cond_broadcast(&(in->cond));
	pthread_mutex_unlock(&(in->lock));
	return;
}

int read_op(struct input_manager *in, op_t *op, int tnum)
{
	unsigned size = sizeof(op_t);
	pthread_mutex_lock(&(in->lock));
	if(in->curr > in->data + in->size){
		pthread_mutex_unlock(&(in->lock));
	    return -1;
	}
	memcpy(op, in->curr, size - sizeof(unsigned long));  //Copy till data ptr     
	if(op->op_type == GET || op->op_type == DEL){
	   in->curr += size - sizeof(op->datalen) - sizeof(op->data);
	}else if(op->op_type == PUT){
	   in->curr += size - sizeof(op->data);
	   op->data = in->curr;
	   in->curr += op->datalen;
	}else{
	   assert(0);
	}
	if(in->curr > in->data + in->size){
		pthread_mutex_unlock(&(in->lock));
	    return -1;
	}

	in->being_processed[tnum-1] = op;
	int flag=0;
	op_t *temp;
	for(int i=0;i<32;i++){
		temp = in->being_processed[i];
		if(temp && temp->key == op->key && temp->id < op->id){
			flag = 1;
			break;
		}
	}

	while(flag){
		pthread_cond_wait(&(in->cond), &(in->lock));
		flag = 0;
		for(int i=0;i<32;i++){
			temp = in->being_processed[i];
			if(temp && temp->key == op->key && temp->id < op->id){
				flag = 1;
				break;
			}
		}
	}

	pthread_mutex_unlock(&(in->lock));

	return 0;
}

int lookup(hash_t *h, op_t *op)
{
	unsigned ctr;
	int cnt=0;
	unsigned hashval = hashfunc(op->key, h->table_size);
	hash_entry_t *entry = h->table + hashval;
	ctr = hashval;
	pthread_mutex_lock(&(entry->lock));
	while((entry->key || entry->id == (unsigned) -1) && 
	     entry->key != op->key && (ctr != hashval || cnt == 0)){
		pthread_mutex_unlock(&(entry->lock));
		if(cnt==0) cnt++;
		ctr = (ctr + 1) % h->table_size;
		entry = h->table + ctr;
		pthread_mutex_lock(&(entry->lock)); 
	} 
	if(entry->key == op->key){
	  op->datalen = entry->datalen;
	  op->data = entry->data;
	  pthread_mutex_unlock(&(entry->lock));
	  return 0;
	}
	pthread_mutex_unlock(&(entry->lock));
	return -1;    // Failed
}

int insert_update(hash_t *h, op_t *op)
{
	unsigned ctr;
	int cnt=0;
	unsigned hashval = hashfunc(op->key, h->table_size);
	hash_entry_t *entry = h->table + hashval;

	assert(h && h->used < h->table_size);
	assert(op && op->key);

	ctr = hashval;
	pthread_mutex_lock(&(entry->lock));

	while((entry->key || entry->id == (unsigned) -1) && entry->key != op->key && (ctr != hashval || cnt == 0)){
		pthread_mutex_unlock(&(entry->lock));
		if(cnt==0) cnt++;
		ctr = (ctr + 1) % h->table_size;
		entry = h->table + ctr;
		pthread_mutex_lock(&(entry->lock)); 
	} 

	assert(ctr != hashval || cnt==0);

	if(entry->key == op->key){  //It is an update
	  entry->id = op->id;
	  entry->datalen = op->datalen;
	  entry->data = op->data;
	  pthread_mutex_unlock(&(entry->lock));
	  return 0;
	}
	assert(!entry->key);   // Fresh insertion

	entry->id = op->id;
	entry->datalen = op->datalen;
	entry->key = op->key;
	entry->data = op->data;
	atomic_add(&(h->used), 1);
	pthread_mutex_unlock(&(entry->lock));
	return 0;
}

int purge_key(hash_t *h, op_t *op)
{
	unsigned ctr;
	int cnt=0;
	unsigned hashval = hashfunc(op->key, h->table_size);
	hash_entry_t *entry = h->table + hashval;

	ctr = hashval;
	pthread_mutex_lock(&(entry->lock));
	while((entry->key || entry->id == (unsigned) -1) && 
	      entry->key != op->key && (ctr != hashval || cnt == 0)){
		pthread_mutex_unlock(&(entry->lock));
		if(cnt==0) cnt++;
		ctr = (ctr + 1) % h->table_size;
		entry = h->table + ctr;
		pthread_mutex_lock(&(entry->lock)); 
	} 

	if(entry->key == op->key){  //Found. purge it
	  entry->id = (unsigned) -1;  //Empty but deleted
	  entry->key = 0;
	  entry->datalen = 0;
	  entry->data = NULL;
	  atomic_add(&(h->used), -1);
	  pthread_mutex_unlock(&(entry->lock));
	  return 0;
	}
	pthread_mutex_unlock(&(entry->lock));
	return -1;    // Bogus purge
}

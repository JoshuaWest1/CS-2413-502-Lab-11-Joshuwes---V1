/* program to demonstrate implementing a linked list in a relative organization binary file
 *
 * the binary file stores a head node and then the restaurant records linked in alphabetical
 *   order
 *
 * to practice:
 *   create a primary index in a separate binary file that would enable someone to find
 *     a restaurant faster than traversing the linked list, such as a hashed primary index
 *   try using linear probing and hashing with buckets for collision resolution
 *   write a hash function for the restaurant names
 *   write a function to insert a restaurant into the hash table
 *   write a function to delete a restaurant from the hash table
 *   write a function to print the hash table file
 *   write a function to print the hash table
 *   write a function to input the hash table from a binary file
 *   write a function to output the hash table to a binary file
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 21               /* restaurant name size */

typedef struct list_node {    /* linked list of restaurants in a relative organization file */
  int node_id;                /* file index */
  int next;                   /* next restaurant */
  char name[SIZE];            /* restaurant name */
} list_node_t;

typedef struct head_node {    /* first record in a relative organization file */
  int size;                   /* number of records in the file */
  int start;                  /* index of first node in the linked list */
  int avail;                  /* index of first node in available node list */
} head_node_t;

typedef union file_node {     /* allows both the head and list nodes to have the same record size */
  list_node_t l_node;         /* all records after the first record */
  head_node_t h_node;         /* first record */
} file_node_t;

int insert_file (FILE *fileb, char name[]); /* insert node into the linked list */
int delete_file (FILE *fileb, char name[]); /* delete node in the linked list */
void print_list (FILE *fileb);              /* print the linked list */
void print_file (FILE *fileb);              /* print the contents of the binary file */
/* utility functions to position the file pointer and read or write a record to the binary file */
void read_record (FILE *fileb, file_node_t *hold, int pos, int whence);
void write_record (FILE *fileb, file_node_t hold, int pos, int whence);

/* demonstrate the relative file organization with a linked list
 *   of restaurants kept in alphabetical order
 */

 
int main (void) {
  /* open the binary file for writing and reading */
  FILE *fileb;
  if ((fileb = fopen("names.bin","wb+")) == NULL) {
    perror ("Cannot open names.bin - exiting...");
    exit(1);
  }
  /* write out the header to the file with 0 records, an empty list, and an empty avail list */
  file_node_t hold;
  head_node_t header = {0, -1, -1};
  hold.h_node = header;
  write_record(fileb, hold, 0, SEEK_SET);
  /* insert the records into the file so that the restaurants are in alphabetical order (linked list) */
  int status;
  printf ("\nInserting appleby's\n");
  status = insert_file(fileb,"appleby's");
  if (!status) {
    printf("DUPLICATE - %s not inserted\n","appleby's");
  }
  print_file(fileb);

  printf ("\nInserting pei wei...\n");
  status = insert_file(fileb,"pei wei");
  if (!status) {
    printf("DUPLICATE - %s not inserted\n","pei wei");
  }
  print_file(fileb);

  printf ("\nInserting double dave's...\n");
  status = insert_file(fileb,"double dave's");
  if (!status) {
    printf("DUPLICATE - %s not inserted\n","double dave's");
  }
  print_file(fileb);

  printf ("\nInserting cracker barrel...\n");
  status = insert_file(fileb,"cracker barrel");
  if (!status) {
    printf("DUPLICATE - %s not inserted\n","cracker barrel");
  }
  print_file(fileb);

  printf ("\nInserting brannigan's...\n");
  status = insert_file(fileb,"brannigan's");
  if (!status) {
    printf("DUPLICATE - %s not inserted\n","brannigan's");
  }
  print_file(fileb);

  printf ("\nInserting pei wei again...\n");
  status = insert_file(fileb,"pei wei");
  if (!status) {
    printf("DUPLICATE - %s not inserted\n","pei wei");
  }
  
  printf("\nAfter 5 Insertions - Print the Linked List\n");
  print_list(fileb);

  /* Delete restaurants from the linked list */
  printf ("\nDeleting appleby's...\n");
  status = delete_file(fileb,"appleby's");
  if (!status) {
    printf("MISSING FROM LIST - %s not deleted\n","appleby's");
  }
  print_file(fileb);

  printf ("\nDeleting double dave's...\n");
  status = delete_file(fileb,"double dave's");
  if (!status) {
    printf("MISSING FROM LIST - %s not deleted\n","double dave's");
  }
  print_file(fileb);

    printf ("\nDeleting appleby's again...\n");
  status = delete_file(fileb,"appleby's");
  if (!status) {
    printf("MISSING FROM LIST - %s not deleted\n","appleby's");
  }

  printf("\nAfter 2 Deletions - Print the Linked List\n");
  print_list(fileb);

  /* Insert a restaurant into the linked list */
  printf ("\nInserting abuelo's...\n");
  status = insert_file(fileb,"abuelo's");
  if (!status) {
    printf("DUPLICATE - %s not inserted\n","abuelo's");
  }
  print_file(fileb);

  printf("\nAfter 1 Insertion - Print the Linked List\n");
  print_list(fileb);

  fclose(fileb);

  return (0);
}

/* insert a node into the linked list in the binary file
 */
int insert_file (FILE *fileb, char name[]) {
  int inserted = 0;                           /* used to indicate successful insertion */
  file_node_t hold;                           /* holds a file record */
  /* input the file header */
  head_node_t header;
  read_record(fileb, &hold, 0, SEEK_SET);
  header = hold.h_node;
  /* search for the insertion position */
  /* current will index the current node read from the file in linked list order */
  /* previous will index the node prior to the insertion point */
  int previous = -1;
  int current = -1;
  current = header.start;
  list_node_t current_node = {0,-1,""};
  list_node_t previous_node = {0,-1,""};
  while (current != -1 && strcmp(name,current_node.name) > 0) {
    read_record(fileb, &hold, current, SEEK_SET);
    current_node = hold.l_node;
    if (strcmp(name,current_node.name) > 0) {
      previous = current;
      previous_node = current_node;
    }
    current = current_node.next;
  }
  /* check for a duplicate key */
  if (strcmp(name,current_node.name) != 0) {
    /* set up the new node to place at the first available node or the end of the file */
    int node_id = header.size+1;                   /* presume new node written to end of file */
    if (header.avail != -1) {                      /* check available list */
      node_id = header.avail;
      read_record(fileb,&hold,node_id,SEEK_SET);
      header.avail = hold.l_node.next;
    }
    else
      header.size++;                  /* increase number of file nodes if avail list is empty */
    list_node_t new_node = {node_id,-1,""};
    strcpy(new_node.name,name);
    /* insertion at head */
    if (previous == -1) {
      new_node.next = header.start;
      header.start = new_node.node_id;
    }
    /* insertion within or at end of list */
    else {
      new_node.next = previous_node.next;
      previous_node.next = new_node.node_id;
      /* write the previous node */
      hold.l_node = previous_node;
      write_record(fileb, hold, previous_node.node_id, SEEK_SET);
    }
    /* write the header node */
    hold.h_node = header;
    write_record(fileb,hold,0,SEEK_SET);
    /* write the new node to the file */
    hold.l_node = new_node;
    write_record(fileb,hold,new_node.node_id,SEEK_SET);
    /* indicate a successful insertion */
    inserted = 1;
  }
  return(inserted);
}

/* delete a node from the linked list in the binary file
 */
int delete_file (FILE *fileb, char name[]) {
  int deleted = 0;                           /* used to indicate successful deletion */
  file_node_t hold;                          /* holds a file record */
  /* input the file header */
  head_node_t header;
  read_record(fileb, &hold, 0, SEEK_SET);
  header = hold.h_node;
  /* search for the deletion position */
  /* current will index the current node read from the file in linked list order */
  /* previous will index the node prior to the deletion point */
  int previous = -1;
  int current = -1;
  current = header.start;
  list_node_t current_node = {0,-1,""};
  list_node_t previous_node = {0,-1,""};
  while (current != -1 && strcmp(name,current_node.name) > 0) {
    read_record(fileb, &hold, current, SEEK_SET);
    current_node = hold.l_node;
    if (strcmp(name,current_node.name) > 0) {
      previous = current;
      previous_node = current_node;
    }
    current = current_node.next;
  }
  /* check for a existence of the key in the list */
  if (strcmp(name,current_node.name) == 0) {
    /* deletion at head */
    if (previous == -1) {
      header.start = current_node.next;
    }
    /* deletion within or at end of list */
    else {
      previous_node.next = current_node.next;
      /* write the previous node */
      hold.l_node = previous_node;
      write_record(fileb, hold, previous_node.node_id, SEEK_SET);
    }
    /* update the avail list */
    current_node.next = header.avail;
    header.avail = current_node.node_id;
    /* write the header node */
    hold.h_node = header;
    write_record(fileb,hold,0,SEEK_SET);
    /* write the current node */
    hold.l_node = current_node;
    write_record(fileb,hold,current_node.node_id,SEEK_SET);
    /* indicate a successful deletion */
    deleted = 1;
  }
  return(deleted);
}

/* print the linked list in the binary file
 */
void print_list (FILE *fileb) {
  file_node_t hold;                            /* holds a file record */
  /* read the header node */
  head_node_t header;
  read_record(fileb, &hold, 0, SEEK_SET);
  header = hold.h_node;
  /* traverse the linked list in the binary file */
  list_node_t current_node;
  int current = header.start;
  while (current != -1) {
    read_record(fileb,&hold,current,SEEK_SET);
    current_node = hold.l_node;
    printf ("NODE %d\n  name: %s\n  next: %d\n",current_node.node_id,current_node.name, current_node.next);
    current = current_node.next;
  }
}

/* print the contents of the binary file
 */
void print_file (FILE *fileb) {
  file_node_t hold;                            /* holds a file record */
  /* read the header node */
  head_node_t header;
  read_record(fileb, &hold, 0, SEEK_SET);
  header = hold.h_node;
  /* print the header node */
  printf("FILE HEADER:\n");
  printf("  number of nodes:   %d\n",header.size);
  printf("  first node index:  %d\n",header.start);
  printf("  avail node index:  %d\n",header.avail);
  /* read the linked list nodes in sequential order from the binary file and print them */
  int i;
  for (i = 1; i <= header.size; i++) {
    read_record(fileb,&hold,0,SEEK_CUR);
    printf ("NODE %d\n  name: %s\n  next: %d\n",hold.l_node.node_id,hold.l_node.name, hold.l_node.next);
  }
}

/* read a record from the binary file
 */
void read_record (FILE *fileb, file_node_t *hold, int pos, int whence) {
  fseek(fileb, pos*sizeof(file_node_t), whence);  /* move the file pointer to the desired record */
  fread(hold,1,sizeof(file_node_t),fileb);        /* read the record into hold */
}

/* write a record to the binary file
 */
void write_record (FILE *fileb, file_node_t hold, int pos, int whence) {
  fseek(fileb, pos*sizeof(file_node_t), whence);  /* move the file pointer to the desired record */
  fwrite(&hold,1,sizeof(file_node_t),fileb);      /* write hold to the file */
}
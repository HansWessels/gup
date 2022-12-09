CC=gcc
CFLAGS=-Ofast 

depack_test: depack_t.c arj_crc.c arj_m4.c arj_m7.c ni_n0.c ni_n1.c ni_n2.c unstore.c
	$(CC) $(CFLAGS) -o depack_test depack_t.c arj_crc.c arj_m4.c arj_m7.c ni_n0.c ni_n1.c ni_n2.c unstore.c

clean:
	rm depack_test

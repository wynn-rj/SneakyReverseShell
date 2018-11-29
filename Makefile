all: server client lkm
	cd server; $(MAKE)
	cd client; $(MAKE)
	cd lkm; $(MAKE)

clean:
	cd server; $(MAKE) clean
	cd client; $(MAKE) clean
	cd lkm; $(MAKE) clean

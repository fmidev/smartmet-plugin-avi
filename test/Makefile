
TEST_PREPARE_TARGETS := cnf/authengine.conf cnf/aviengine.conf
TEST_FINISH_TARGETS := dummy
TEST_TARGETS := test-noauth test-auth

ifdef CI
GEONAMES_HOST_EDIT := sed -e 's/"smartmet-test"/"localhost"/g'
TEST_PREPARE_TARGETS += start-geonames-db
TEST_FINISH_TARGETS += stop-test-db
EXTRA_IGNORE := input/.testignore_circle-ci
else
GEONAMES_HOST_EDIT := cat
EXTRA_IGNORE :=
endif

TEST_DB_DIR := $(shell pwd)/tmp-geonames-db
TEST_RUNNER :=

all:

clean:
	-$(MAKE) $(TEST_FINISH_TARGETS)
	rm -rf failed-sqlite failed-oracle failed-postgresql
	rm -rf tmp-geonames-db

test:
	@ok=true; failed=; \
	for test in $(TEST_TARGETS); do \
	  if ! $(MAKE) $$test; then ok=false; failed="$$failed $$test"; fi; \
        done; \
	if ! $$ok ; then \
	  echo "======================================================================="; \
	  echo "Failed: $$failed"; \
	  echo "======================================================================="; \
        fi; \
        $$ok

test-noauth:		CONFIG := cnf/reactor.conf
test-auth:		CONFIG := cnf/reactor-with-authentication.conf

test-noauth test-auth:
	$(MAKE) $(TEST_PREPARE_TARGETS)
	rm -rf failures-$@
	mkdir -p failures-$@
	ok=false; $(TEST_RUNNER) smartmet-plugin-test --handler /avi \
		--reactor-config $(CONFIG) \
		--failures-dir failures-$@ \
		$(foreach fn, input/.testignore $(EXTRA_IGNORE), --ignore $(fn)) \
		--timeout 300 && ok=true; \
	$(MAKE) $(TEST_FINISH_TARGETS); \
	$$ok

geonames-database:
	@-$(MAKE) stop-test-db
	rm -rf tmp-geonames-db
	if ! /usr/share/smartmet/test/db/create-local-db.sh $(TEST_DB_DIR) >tmp-geonames-db.log 2>&1 ; then \
	    cat tmp-geonames-db.log; \
	    false; \
	fi

start-geonames-db: geonames-database
	/usr/share/smartmet/test/db/test-db-ctl.sh $(TEST_DB_DIR) start -w

stop-test-db:
	-/usr/share/smartmet/test/db/test-db-ctl.sh $(TEST_DB_DIR) stop

cnf/authengine.conf cnf/aviengine.conf:	dummy
	$(GEONAMES_HOST_EDIT) $@.in >$@

dummy:

.PHONY: cnf/authengine.conf cnf/aviengine.conf

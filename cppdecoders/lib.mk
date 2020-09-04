define BUILD_LIBRARY
$(if $(wildcard $@),@rm $@)
$(if $(wildcard ar.mac),@rm ar.mac)
$(if $(filter lib%.a, $^),
@echo CREATE $@ > ar.mac
@echo SAVE >> ar.mac
@echo END >> ar.mac
@ar -M < ar.mac
)
$(if $(filter %.o,$^),@ar -q $@ $(filter %.o, $^))
$(if $(filter %.a, $^),
@echo OPEN $@ > ar.mac
$(foreach LIB, $(filter %.a, $^),
@echo ADDLIB $(LIB) >> ar.mac
)
@echo SAVE >> ar.mac
@echo END >> ar.mac
@ar -M < ar.mac
@rm ar.mac
)
endef

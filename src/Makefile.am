bin_PROGRAMS = telecom

telecom_SOURCES = \
   gsm/state_machine/base.cc \
   gsm/state_machine/base.h \
   gsm/state_machine/state.h \
   gsm/state_machine/burst.cc \
   gsm/state_machine/burst.h \
   gsm/state_machine/find_fcch.cc \
   gsm/state_machine/find_fcch.h \
   gsm/state_machine/synchronize.cc \
   gsm/state_machine/synchronize.h \
   gsm/analyzer.cc \
   gsm/analyzer.h \
   base_analyzer.cc \
   base_analyzer.h \
   telecom_analyzer.cc


telecom_CXXFLAGS = -I../lib/kalibrate-hackrf/src $(FFTW3_CFLAGS) $(LIBHACKRF_CFLAGS)
telecom_LDADD = ../lib/libkalibrate-hackrf.a $(FFTW3_LIBS) $(LIBHACKRF_LIBS)

AR=lib /nologo
ARFLAGS=/out:cppre.lib
CC=cl /nologo
CXXFLAGS=-I. -c -Ox -EHsc
# CXXFLAGS=-I. -c -Zi -EHsc

SRC=          \
Matcher.cpp   \
Pattern.cpp   \
WCMatcher.cpp \
WCPattern.cpp \

OBJ=$(SRC:.cpp=.obj)

cppre.lib: $(OBJ)
	$(AR) $(ARFLAGS) $(OBJ)

test: cppre.lib test.cpp
	$(CC) $(CXXFLAGS) test.cpp -D_CRT_SECURE_NO_DEPRECATE=1
	$(CC) -Fetest.exe -Zi -EHsc test.obj cppre.lib

install:
	xcopy /Y /F cppre.lib "$(MSVCDIR)\lib"
	xcopy /Y /E /I /F regexp "$(MSVCDIR)\include\regexp"

.cpp.obj:
	$(CC) $(CXXFLAGS) $<

clean:
	del $(OBJ) *~ cppre.lib *.ilk *.pdb *.obj test.exe

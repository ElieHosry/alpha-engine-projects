::CMD will no longer show us what command it’s executing(cleaner)
@ECHO OFF 

::Title of the window
TITLE AEEngine Doxygen Batchfile

::Display starting message
ECHO\
ECHO Batch file for generating doxygen documentation for the AE game engine.

::Begin generating documentation
ECHO Begin generating documentation...
ECHO\

doxygen AEDoxyfile
ECHO Completed.

::Documentation generation completed
ECHO\
ECHO Generation of documentation completed.


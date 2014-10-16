# Created by IBM WorkFrame/2 MakeMake at 08:29:30 on 01/13/94
#
# This makefile should be run in the following directory:
#   H:\IBMWF\PRJ\RSQL
#
# The actions included in this makefile are:
#   COMPILE::CLC C++
#   LINK::Link

.all: \
  .\RSQL.EXE \
  RSQL.MAK

RSQL.MAK:  C:\DESKTOP\MAIN\DEVELOP\WORKFRAME!2\PROJECTS\RSQL
    -@echo Warning: If any action options have changed,
    -@echo you should re-run MakeMake.

.SUFFIXES:

.SUFFIXES: .sqc .c

.SQC.C:
      @echo WF::SQLPREP::DB2/2
      sqlprep.exe %s SAMPLE /FISO /ICS

.c.obj:
      @echo WF::COMPILE::CLC C++
      icc.exe /DINCL_32 /Tl- /Sm /Sp1 /J- /Q /Ti /Gm /Gd /G4 /Gs /Ft- /C %s

.\RSQL.EXE: \
    .\RSQL.obj \
    {$(LIB)}sql_dyn.lib \
    {$(LIB)}upm.lib \
    RSQL.MAK
      @echo WF::LINK::Link
      link386.exe @<<
         /PM:VIO /DE /NOP /NOI /NOE /NOLOGO /M:FULL /L /ST:64000 +
        .\RSQL.obj
        RSQL.EXE
        
        sql_dyn +
        upm
        ;
<<


!include RSQL.Dep

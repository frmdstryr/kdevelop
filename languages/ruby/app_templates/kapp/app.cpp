#include <ruby.h>

#include <qstring.h>
#include <qfileinfo.h>
#include <kinstance.h>
#include <kstandarddirs.h>

int main(int argc, char **argv) {
    KInstance * instance = new KInstance("%{APPNAMELC}");
    QString appdir = ::locate("data", "%{APPNAMELC}/main.rb", instance);
    QFileInfo program(appdir);
     
    char ** rubyargs = (char **) calloc(5, sizeof(char *));
    rubyargs[0] = strdup(argv[0]);
    rubyargs[1] = strdup("-KU");
    rubyargs[2] = strdup(QString("-C%1").arg(program.dirPath()).latin1());
    rubyargs[3] = strdup(QString("-I%1").arg(program.dirPath()).latin1());
    rubyargs[4] = strdup(program.fileName().latin1());
          
    ruby_init();
    ruby_options(argc + 4, rubyargs); 
    ruby_script(strdup("%{APPNAMELC}"));
    ruby_run();
}

// util.h

typedef struct _OPTIONS{
}OPTIONS;

class MyIniFile
{
public:
  MyIniFile() {};
  virtual ~MyIniFile() {};

  virtual void load(char *filename, OPTIONS * /*options*/)
  {
    FILE *cfg=fopen(filename, "r");
    if(cfg!=NULL){
      const int nlinebuf=1024;
      char line[nlinebuf];
      memset(line, 0, nlinebuf);
      while(fgets(line, sizeof(line), cfg)){
        // char *found=0;
        //        if(strstr(line, "DataFolder")){
        //          found=strchr(line, '=')+1;
        //          sscanf(found, "%s", options->dataFolderName);
        //        }else if(strstr(line, "IntTest")){
        //          found=strchr(line, '=')+1;
        //          sscanf(found, "%d", &(options->inttest));
        //        }
        memset(line, 0, nlinebuf);
      }
    }
    fclose(cfg);
  };

  virtual void save(char *filename, OPTIONS * /*options*/)
  {
    FILE *cfg=fopen(filename, "w");
    if(cfg!=NULL){
      //      fprintf(cfg, "DataFolder=%s\n", options->dataFolderName);
      //      fprintf(cfg, "IntTest=%d\n", options->inttest);
      fclose(cfg);
    }
  };
};

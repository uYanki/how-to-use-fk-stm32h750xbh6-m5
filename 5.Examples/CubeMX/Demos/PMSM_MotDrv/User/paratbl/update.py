

import re
import os
import shutil

CONFIG_PARATBL_PTAH = "./paratbl.h" 

filesrc = CONFIG_PARATBL_PTAH
filedst = filesrc

# regex
pat_grp = re.compile(r"typedef struct __packed \{(.*?)\} (.*?_t);", re.S)
pat_para = re.compile(r"([s|u|f](\d{2}) .*?(\[\d{1,}\])?; {0,}///<)( ?P\d{1,})?")

map_size = {"16":1,"32":2,"64":4}

if os.path.exists(filesrc):

    ctx = ""

    if filedst == filesrc:  # backup
        backup_index = 1
        backup_filename = ""
        while True:
            backup_filename = filesrc + "." + str(backup_index) + ".bak"
            if not os.path.exists(backup_filename):
                break
            backup_index += 1
        shutil.copyfile(filesrc, backup_filename)


    with open(filesrc, "r", encoding = "utf-8") as fsrc:
        ctx = fsrc.read()
        
    os.remove(filesrc)

    with open(filedst, "w+", encoding = "utf-8") as fdst:

        for grps in pat_grp.findall(ctx):

            para_offset_new = 0

            for paras in pat_para.findall(grps[0]):

                para_define = paras[0]
                para_size = paras[1]
                para_array_size = paras[2]
                para_offset_old = paras[3]

                para_oldline = para_define + para_offset_old
                para_newline = para_define + (" P%04d" % para_offset_new)

                if para_array_size == '':
                    para_offset_new += map_size[para_size]
                else:
                    para_offset_new += map_size[para_size] * int(para_array_size[1:-1])

                print(para_oldline,para_newline)

                ctx = ctx.replace(para_oldline, para_newline,1)

        fdst.writelines(ctx)



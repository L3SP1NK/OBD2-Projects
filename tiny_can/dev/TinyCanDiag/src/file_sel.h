#ifndef __FILE_SEL_H__
#define __FILE_SEL_H__

#define FILE_SELECTION_OPEN         0
#define FILE_SELECTION_READ_BROWSE  1
#define FILE_SELECTION_SAVE         2
#define FILE_SELECTION_WRITE_BROWSE 3

int SelectFileDlg(const gchar *title, unsigned char action, char **file_name);

#endif

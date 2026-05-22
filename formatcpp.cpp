//strcpy    _tcscpy
//strcmp    _tcscmp

//strcpy    _tcscpy   wcscpy    _mbscpy
//strcmp    _tcscmp   wcscmp    _mbscmp

#include "perm.h"

#ifdef _UNICODE
#define printf __utf8_printf
#ifdef _tprintf
#undef _tprintf
#endif
#define _tprintf __unicode_printf
#endif


#ifdef __unix__

#define _T(x) x
#define WndSet(x) x

#ifdef _tprintf
#undef _tprintf
#endif
#define _tprintf __utf8_printf

#ifdef printf
#undef printf
#endif
#define printf __utf8_printf

#endif

const char * gpsz = NULL;

CMaaString crlf =
#ifdef __unix__
"\n"
#else
"\r\n"
#endif
;

int FormatCppSourceFile(CMaaString FileName)
{
    int nTabSpaces =
#ifdef __unix__
    4
#else     
    4 // 5
#endif
    ;
    printf("%S...", &FileName);

    int nSlash = FileName.ReverseFind(FILESYSTEM_SLASH);
    CMaaString Dir = nSlash >= 0 ? FileName.Left(nSlash + 1) : "";
    CMaaString FileNameWinOutDir = FileName.Mid(nSlash >= 0 ? nSlash + 1 : 0);

    if  (FileNameWinOutDir.ToLower() == "resource.h")
    {
        printf(" - skipped\n");
        return 0;
    }

    CMaaString BackupFileName = Dir + "temp\\bk\\" + FileNameWinOutDir;
    CMaaString NewFileName =  Dir + "temp\\new\\" + FileNameWinOutDir;
    try
    {
        CMaaFile::CopyFile(FileName, BackupFileName, CMaaFile::eCreateFolder, true);

        CMaaFile fsrc(FileName, "R|SrSw", true);
        _qword SrcFileLength = fsrc.Length();
        CMaaFile fdst(NewFileName, "WCD|SrSw", true);

        int LineNum = 0;
        int nConsequentErrors = 0, ncquotes = 0;

        int prev_pos = 0, prev_1st = 0, nRoundBrackets = 0, nSquareBrackets = 0, nDblQuotes = 0, nSingleQuotes = 0, nBrackets = 0, nClass = 0;
        int n_c_comment = 0, n_cpp_comment = 0;
        //int nreal_pos = 0;
        CMaaString previous_str_part;
        while(1)//previous_str_part.Length() > 0 || fsrc.IsOpen())
        {
            int i = 0;
            if  (!n_c_comment)
            {
                for (i = 0; i < previous_str_part.Length(); i++)
                {
                    if  (previous_str_part[i] != ' ')
                    {
                        break;
                    }
                }
                if  (i > 0 && i >= previous_str_part.Length())
                {
                    previous_str_part.Empty();
                    fdst.Write(crlf);
                }
            }
            bool bContinueString = previous_str_part.Length() > 0;
            CMaaString str = bContinueString ? previous_str_part : fsrc.fgets(1024 * 1024);
            if  (!fsrc.IsOpen() && str.Length() == 0)
            {
                break;
            }
            /*
               if   (str.Right(7) == "* m_px;")
               {
                    static int aa = 0;
                    aa++;
                    aa++;
                    aa++;
               }
               */
            previous_str_part.Empty();
            CMaaConcatString line(str.Length() + 1024, 1);
            int pos1st = 0;
            while(str[pos1st] == ' ' || str[pos1st] == '\t')
            {
                pos1st++;
            }
            int poslast = str.Length();
            while(poslast > pos1st && (str[poslast - 1] == ' ' || str[poslast - 1] == '\t'))
            {
                poslast--;
            }
            if  (n_c_comment)
            {
                int nendpos = str.Find(pos1st, "*/", 2);
                if  (nendpos >= 0)
                {
                    previous_str_part = str.Mid(nendpos + 2);
                    str = str.Left(nendpos + 2);
                    n_c_comment = 0;
                }
            }
            else if (str[pos1st] != '#')
            {
                int nBrackets_1 = nBrackets + nClass;
                int AnyOpened = (nRoundBrackets | nSquareBrackets | nDblQuotes | nSingleQuotes);
                for (int i = pos1st; i < poslast && previous_str_part.Length() == 0; )
                {
                    int tmp_i = i;
                    char c = str[i++];
                    if  ((nDblQuotes | nSingleQuotes) != 0)
                    {
                        line.Add(&c, 1);
                        switch(c)
                        {
                        case '\\':
                            if  (i < poslast)
                            {
                                c = str[i++];
                                line.Add(&c, 1);
                            }
                            break;
                            /*
                              case '/':
                                   if   (i < poslast && str[i] == '/')
                                   {
                                        c = str[i++];
                                        line.Add(&c, 1);
                                        line.Add(i + (const char *)str, poslast - i);
                                        i = poslast;
                                   }
                                   break;
*/
                        case '\"':
                            if  (nSingleQuotes)
                            {
                            }
                            else
                            {
                                nDblQuotes = nDblQuotes ^ 1;
                            }
                            break;
                        case '\'':
                            if  (!nDblQuotes)
                            {
                                nSingleQuotes = nSingleQuotes ^ 1;
                            }
                            break;
                        }
                    }
                    else
                    {
                        switch(c)
                        {
                        case '/':
                            line.Add(&c, 1);
                            if  (i < poslast && str[i] == '*')
                            {
                                c = str[i++];
                                line.Add(&c, 1);
                                previous_str_part = str.Mid(i);
                                n_c_comment++;
                            }
                            else if (poslast - i > 13 && str.RefMid(i - 1, 14) == "// Format C++ ")
                            {
                                line += str.RefMid(i, 13);
                                i += 13;
                            }
                            else if (i < poslast && str[i] == '/')
                            {
                                line += str.Mid(i, poslast - i);
                                i = poslast;
                                //previous_str_part.Empty();
                            }
                            break;
                        case '(':
                            line.Add(&c, 1);
                            nRoundBrackets++;
                            break;
                        case ')':
                            line.Add(&c, 1);
                            nRoundBrackets--;
                            break;
                        case '[':
                            line.Add(&c, 1);
                            nSquareBrackets++;
                            break;
                        case ']':
                            line.Add(&c, 1);
                            nSquareBrackets--;
                            break;
                        case '\"':
                            line.Add(&c, 1);
                            nDblQuotes = nDblQuotes ^ 1;
                            break;
                        case '\'':
                            line.Add(&c, 1);
                            nSingleQuotes = nSingleQuotes ^ 1;
                            break;
                        case '{':
                            line.Add(&c, 1);
                            nBrackets++;
                            if  (tmp_i == pos1st)
                            {
                                nBrackets_1 -= nClass;
                            }
                            nClass = 0;
                            break;
                        case '}':
                            line.Add(&c, 1);
                            nBrackets--;
                            break;
                        case '\\':
                            line.Add(&c, 1);
                            //break;//opt
                            if  (i < poslast)
                            {
                                c = str[i++];
                                line.Add(&c, 1);
                            }
                            break;
                        case ':':
                            line.Add(&c, 1);
                            if  (str[i] == ':')
                            {
                                line += ':';
                                i++;
                                break;
                            }
                            if  (tmp_i == pos1st)
                            {
                                while(str[i] == ' ' || str[i] == '\t' && i < poslast)
                                {
                                    i++;
                                }
                                if  (i < poslast)
                                {
                                    CMaaString sp(NULL, nTabSpaces - 1);
                                    sp.Fill(' ');
                                    line += sp;
                                }
                                nClass = 1;
                            }
                            break;
                        default:
                            if  (tmp_i == 0 || (str[tmp_i - 1] == ' ' || str[tmp_i - 1] == '\t'))
                            {
                                int j;
                                CMaaString word;
                                for (j = tmp_i; j < poslast; j++)
                                {
                                    if  ((str[j] >= 'a' && str[j] <= 'z') ||
                                         (str[j] >= 'A' && str[j] <= 'Z') ||
                                         (j > i && (str[j] >= '0' && str[j] <= '9'))
                                    )
                                    {
                                        // valid char of the word identificator
                                    }
                                    else if (str.Mid(tmp_i, j - tmp_i) == "else")
                                    {
                                        word = str.Mid(tmp_i, j - tmp_i);
                                        int jj = j;
                                        while(str[jj] == ' ' || str[jj] == '\t')
                                        {
                                            jj++;
                                        }
                                        if  (jj != j && str.Mid(jj, 2) == "if")
                                        {
                                            jj += 2;
                                            if  (str[jj] == ' ' || str[jj] == '\t')
                                            {
                                                j = jj;
                                                word = "else if";
                                            }
                                        }
                                        break;
                                    }
                                    else
                                    {
                                        word = str.Mid(tmp_i, j - tmp_i);
                                        break;
                                    }
                                }
                                while(str[j] == ' ' || str[j] == '\t' && j < poslast)
                                {
                                    j++;
                                }
                                if  (str[j] == '(' && tmp_i == pos1st)
                                {
                                    const char * pszKeyWords[] = {"if",  "for", "while", "do", "switch", "catch", "else if", NULL};
                                    const char * pszSpaces[]   = {"  ",  " ",   "",      "",   "",       "",      " ",       NULL};
                                    for (int k = 0; pszKeyWords[k] && pszSpaces[k]; k++)
                                    {
                                        if  (word == pszKeyWords[k])
                                        {
                                            line += word;
                                            line += pszSpaces[k];
                                            i = j;
                                            break;
                                        }
                                    }
                                }
                                else if(tmp_i == pos1st)
                                {
                                    const char * psz_cpp_access_type_modificators[3] = {"private", "protected", "public"};
                                    const char * psz_c_backtabs[2] = {"case", "default"};
                                    for (int k = 0; k < 3; k++)
                                    {
                                        if  (word == psz_cpp_access_type_modificators[k] || (k < 2 && word == psz_c_backtabs[k]))
                                        {
                                            nBrackets_1--;
                                            break;
                                        }
                                    }
                                }
                            }
                            if  (i == tmp_i + 1)
                            {
                                line.Add(&c, 1);
                            }
                        }
                    }
                }
                int pos = nBrackets_1 * nTabSpaces;
                int pos_111 = pos;
                if  (AnyOpened == 0)
                {
                    if  (str[pos1st] == '}')
                    {
                        for (int i = pos1st; i < poslast; i++)
                        {
                            char c = str[i];
                            if  (c == '}')
                            {
                                nBrackets_1--;
                            }
                            else if (c == ' ' || c == '\t')
                            {
                            }
                            else
                            {
                                break;
                            }
                        }
                        pos = nBrackets_1 * nTabSpaces;
                    }
                    pos_111 = pos;
                }
                else
                {
                    pos = prev_pos + (pos1st - prev_1st);
                    pos_111 = pos;// + nTabSpaces;
                    /*
                         printf("%d. %d %d   %d %d, nRoundBrackets=%d, nSquareBrackets=%d, nDblQuotes=%d, nSingleQuotes=%d\n",
                         LineNum + 1, pos, pos_111, bContinueString ? 1 : 0, pos1st, nRoundBrackets, nSquareBrackets, nDblQuotes, nSingleQuotes);
*/
                }

                if  (nRoundBrackets < 0 || nSquareBrackets < 0 || nDblQuotes < 0 || nSingleQuotes < 0)
                {
                    nConsequentErrors++;
                    if  (nConsequentErrors <= 10)
                    {
                        printf("%d. %d %d   %d %d, nRoundBrackets=%d, nSquareBrackets=%d, nDblQuotes=%d, nSingleQuotes=%d\n",
                                       LineNum + 1, pos, pos_111, bContinueString ? 1 : 0, pos1st, nRoundBrackets, nSquareBrackets, nDblQuotes, nSingleQuotes);
                    }
                    else if (nConsequentErrors == 11)
                    {
                        printf("...\n");
                    }
                    if  (nConsequentErrors == 10)
                    {
                        //usleep(500000);
                    }
                }
                else
                {
                    nConsequentErrors = 0;
                    if  ((nDblQuotes | nSingleQuotes))
                    {
                        ncquotes++;
                        if  (ncquotes <= 10)
                        {
                            printf("%d. nDblQuotes=%d, nSingleQuotes=%d\n",
                                LineNum + 1, nDblQuotes, nSingleQuotes);
                        }
                        else if (ncquotes == 11)
                        {
                            printf("...\n");
                        }
                        if  (ncquotes == 10)
                        {
                            //usleep(500000);
                        }
                    }
                    else
                    {
                        ncquotes = 0;
                    }
                }

                pos = pos >= 0 ? pos : 0;
                pos_111 = pos_111 >= 0 ? pos_111 : 0;
                /*
                    if   (pos_111 != pos)
                    {
                         printf(">>%d. %d %d\n", LineNum + 1, pos, pos_111);
                    }
*/
                CMaaString txt(NULL, !bContinueString ? (line.GetLength() == 0 ? 0 : pos_111) : pos1st);
                txt.Fill(' ');
                CMaaString newstr = txt + CMaaString(line);//str.Mid(pos1st, poslast - pos1st);
                //if   (newstr.Length() == pos + poslast - pos1st)
                {
                    str = newstr;
                }

                if  (!bContinueString)
                {
                    prev_1st = pos1st;
                    prev_pos = pos;
                }
            }
            fdst.Write(str);
            if  (previous_str_part.Length() == 0)
            {
                fdst.Write(crlf);
                LineNum++;
            }
        }
        _qword DstFileLength = fdst.Length();
        fdst.Close();
        fsrc.Close();
        CMaaFile::CopyFile(NewFileName, FileName, false, true);
        _qword NewFileLength = CMaaFile::Length(FileName);
        CMaaString DoneMsg;
        if  (NewFileLength == DstFileLength)
        {
            CMaaFile::Remove(NewFileName, false);
            DoneMsg.Format(" - done, %,D bytes (%+,D)\n", NewFileLength, NewFileLength - SrcFileLength);
        }
        else
        {
            DoneMsg.Format(" - done, %,D bytes (%+,D), !!!\n--- !!!Background task has prevented to copy new produced file to the source, produced file length was %,D bytes!!!\n", NewFileLength, NewFileLength - SrcFileLength, DstFileLength);
        }
        printf("%S", &DoneMsg);
    }
    catch(XTOOFile2Error err)
    {
#ifdef _UNICODE
        printf("error: %s\n", err.GetMsg());
#else
        printf("error: %s\n", err.GetMsg());
#endif
        return 0;
    }
    catch(...)
    {
        return 0;
    }
    return 1;
}

int FormatCppSourceFile(CMaaString Mask, int depth)
{
    int nFiles = 0;
    //CMaaFindFile2 ff(WndGet(Mask), 1);
    CMaaFindFile2 ff(Mask, depth);
    CMaaFindFile2::sFind f;
    while(ff.Get(f))
    {
        if  (f.m_Type == CMaaFindFile2::sFind::eFile)
        {
            nFiles += FormatCppSourceFile(f.m_FileName);
        }
    }
    return nFiles;
}

int main(int argn, char * args[])
{
    if  (argn < 2)
    {
        printf("%s filemask1 filemask2 ...\n", args[0]);
        return 0;
    }
    try
    {
        int i;
        for (i = 1; i < argn; i++)
        {
            CMaaString Arg = args[i];
            FormatCppSourceFile(Arg, 1);
        }
    }
    catch(...)
    {
        return 1;
    }
    return 1;
}

auto-generated from `micro.keys` at Sun Oct 28 21:45:20 2012

<div>
<span style="font-family:Consolas,Liberation Mono,Menlo,monospace">
<font size="2">
/*-----------------------------------------------------------------------------<br>
 * µMup07 = МикроМир07 = multi-platform text editor • MacOS Х • Linux • Windows<br>
 * ----------------------------------------------------------------------------<br>
 * original "MicroMir" idea (c) А.Г.Кушниренко, Г.В.Лебедев (мехмат МГУ)&nbsp;&nbsp; 1981<br>
 * "nanoMir" implementation (c) Attic (Д.В.Варсанофьев, А.Г.Дымченко) 1989-1991<br>
 * "nanoMir" and МикроМир07 (c) EpiMG (М.Г.Эпиктетов)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 1998-2001,2006-2012<br>
-------------------------------------------------------------------------------<br>
 Ins is "Help" key on MacPro keyboard, missing on A1243 (see KEY MAPPING below)<br>
 Del is "Delete⌦" on Mac keyboard ("delete" key is Backspace)<br>
 Return is either "Enter↵" or "Return" on main keyboard (not "enter" on numpad)<br>
 PgUp/PgDown on MacBook keyboard may be entered as Fn+Up/Fn+Down (modifiers Ok)<br>
 Alt&nbsp;&nbsp;== ⌥/option&nbsp;&nbsp;key on Mac keyboard, Alt key elsewhere<br>
 Meta == ⌘/command key on Mac keyboard, "WinKey" key on Windows-compatible one<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; (in most cases, Meta+{x} has alias Ctrl+Numpad{x})<br>
COMMAND LINE<br>
&nbsp;&nbsp;&nbsp;&nbsp; PARAMETERS:&nbsp;&nbsp;&nbsp;&nbsp;mim filename&nbsp;&nbsp;&nbsp;&nbsp;- start editing given file<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;mim filename:N: - start editing given file at line N<br>
(see also FILES&nbsp;&nbsp;&nbsp;&nbsp; mim -&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - start editing an empty text<br>
&nbsp;&nbsp; section below)&nbsp;&nbsp; mim dirname&nbsp;&nbsp;&nbsp;&nbsp; - open the contents of given directory<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;mim .&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - start with current directory contents<br>
Options:<br>
 -dos - save files in DOS/Win format (with CR LF) instead of default POSIX (LF)<br>
 -dq&nbsp;&nbsp;- debug memory allocation |<br>
 -ti&nbsp;&nbsp;- show execution timing&nbsp;&nbsp; | not intended for regular use, for debugging<br>
 -kb&nbsp;&nbsp;- show keyboard commands&nbsp;&nbsp;| purposes only, do not use unless instructed<br>
*/<br>
k_BEGIN<br>
/*<br>
<b>First MicroMir principle:</b> text is not a sequence of bytes (as it is represented<br>
in the operating system) but rather a 2-dimensional matrix of lines filled with<br>
characters and cursor (current editing position) located on some character.<br>
<br>
<b>BASIC CURSOR</b>&nbsp;&nbsp;&nbsp;&nbsp;{arrows}&nbsp;&nbsp;&nbsp;&nbsp;- move cursor one position in given direction<br>
&nbsp;&nbsp;<b>MOVEMENT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+Left&nbsp;&nbsp; - move cursor to beginning of the line<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+Right&nbsp;&nbsp;- move cursor to end of the line<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Alt+Up&nbsp;&nbsp; / Alt+Down&nbsp;&nbsp;- move cursor a few lines (1/6th of window) up/down<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Alt+Left / Alt+Right - move cursor word left/right<br>
 ------------------------------------------------------------------------------<br>
 Word = sequence of non-space characters; in addition, '(' starts new word, and<br>
 comma and semicolon terminate the word (all three are included into the word)<br>
*/<br>
k_(LE_LEFT,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Left",&nbsp;&nbsp; 0xd0012) /* курсор влево на одну позицию&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TE_UP,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Up",&nbsp;&nbsp;&nbsp;&nbsp; 0xd0013) /* курсор вверх&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(LE_RIGHT,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Right",&nbsp;&nbsp;0xd0014) /* курсор вправо&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TE_DOWN,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Down",&nbsp;&nbsp; 0xd0015) /* курсор вниз&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_BEG,&nbsp;&nbsp; "Ctrl+Left",&nbsp;&nbsp; 0xd0412) /* курсор в самое начало строки&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(LE_END,&nbsp;&nbsp; "Ctrl+Right",&nbsp;&nbsp;0xd0414) /* - в конец, за последний не-пробел&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TW_UP,&nbsp;&nbsp;&nbsp;&nbsp; "Alt+Up",&nbsp;&nbsp;&nbsp;&nbsp; 0xd0813) /* на несколько строк вверх&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TW_DOWN,&nbsp;&nbsp; "Alt+Down",&nbsp;&nbsp; 0xd0815) /* на несколько строк вниз (1/6ая окна) */<br>
k_(LE_PWORD,&nbsp;&nbsp;"Alt+Left",&nbsp;&nbsp; 0xd0812) /* -&gt; предыдущее слово&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_NWORD,&nbsp;&nbsp;"Alt+Right",&nbsp;&nbsp;0xd0814) /* -&gt; следующее слово&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
/*<br>
&nbsp;&nbsp;&nbsp;&nbsp; PgUp/PgDown&nbsp;&nbsp;&nbsp;&nbsp;- scroll text one page (= windows height - 1) up/down<br>
Ctrl+PgUp/PgDown&nbsp;&nbsp;&nbsp;&nbsp;- move cursor to very beginning / end of the document<br>
Ctrl+Up / Ctrl+Down - move continuously up/down (stop by any key)<br>
Ctrl+[&nbsp;&nbsp;/ Ctrl+]&nbsp;&nbsp;&nbsp;&nbsp;- move cursor to left / right edge of the current window<br>
Meta+Ctrl+{arrows}&nbsp;&nbsp;- scroll text in given direction (cursor moves over text,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;while remaining in the same position relative to window)<br>
*/<br>
k_(TE_PPAGE,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "PgUp",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xd0016) /* на страницу вверх&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ..0 */<br>
k_(TE_NPAGE,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "PgDown",&nbsp;&nbsp;&nbsp;&nbsp; 0xd0017) /* на страницу вниз&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;..1 */<br>
k_(TE_TBEG,&nbsp;&nbsp;&nbsp;&nbsp; "Ctrl+PgUp",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xd0416) /* в самое начало текста&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TE_TEND,&nbsp;&nbsp;&nbsp;&nbsp; "Ctrl+PgDown",&nbsp;&nbsp;&nbsp;&nbsp; 0xd0417) /* в самый конец текста&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TW_CUP,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+Up",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc0413) /* непрерывно вверх&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TW_CDOWN,&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+Down",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc0415) /* непрерывно вниз&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(LE_WLEFT,&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+[",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xd045b) /* в самую левую / правую&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_WRIGHT,&nbsp;&nbsp; "Ctrl+]",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xd045d) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;позицию окна */<br>
k_(TW_SCROLUPN,&nbsp;&nbsp;&nbsp;&nbsp; NULL,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xd00fa) /* (mouse wheel&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TW_SCROLDNN,&nbsp;&nbsp;&nbsp;&nbsp; NULL,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xd00fd) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;scrolling)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TW_SCROLLF,&nbsp;&nbsp;"Meta+Ctrl+Left",&nbsp;&nbsp;0xd1412)<br>
k_(TW_SCROLUP,&nbsp;&nbsp;"Meta+Ctrl+Up",&nbsp;&nbsp;&nbsp;&nbsp;0xd1413) /* NOTE: scoll up/down&nbsp;&nbsp; */<br>
k_(TW_SCROLRG,&nbsp;&nbsp;"Meta+Ctrl+Right", 0xd1414) /*&nbsp;&nbsp;differs in 2nd LSB:&nbsp;&nbsp;*/<br>
k_(TW_SCROLDN,&nbsp;&nbsp;"Meta+Ctrl+Down",&nbsp;&nbsp;0xd1415) /*&nbsp;&nbsp;&nbsp;&nbsp;..1. up, ..0. down */<br>
/*<br>
For window handling, MicroMir uses concept familiar to UNIX programmers: 'fork'<br>
(make a copy of the window) and 'replace file' (although this does not actually<br>
replaces anything, it just pushes current text into stack like Esc &lt;down&gt; does)<br>
<br>
&nbsp;&nbsp;Meta+'+'('=')/F11 - open new window with the same contents as in current one<br>
&nbsp;&nbsp;Meta+'-'&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- split the window into two panes, forking the contents<br>
&nbsp;&nbsp;Meta+Del/F12&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- ask for filename and enter that file (see FILES below)<br>
<br>
&nbsp;&nbsp;Meta+{arrow} - move cursor to closest window in given direction<br>
&nbsp;&nbsp;Meta+Return&nbsp;&nbsp;- SyncPos(tm) - open (in upper pane) file mentioned here<br>
*/<br>
k_(TM_VFORK,&nbsp;&nbsp; "Meta+=",&nbsp;&nbsp; 0xc103d) /* открыть новое окно с тем&nbsp;&nbsp; */<br>
a_(TM_VFORK,&nbsp;&nbsp; "Meta+F11", 0xc103d) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;же самым содержимым */<br>
a_(TM_VFORK,&nbsp;&nbsp; "Ctrl+[+]", 0xc103d) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TM_HFORK,&nbsp;&nbsp; "Meta+-",&nbsp;&nbsp; 0xc102d) /* поделить окно пополам&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(TM_HFORK,&nbsp;&nbsp; "Ctrl+[-]", 0xc102d) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TM_FENTR,&nbsp;&nbsp; "Meta+Del", 0xc1007) /* ввести имя и войти&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
a_(TM_FENTR,&nbsp;&nbsp; "Meta+F12", 0xc1007) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; в указанный файл */<br>
a_(TM_FENTR,&nbsp;&nbsp; "Ctrl+[*]", 0xc1007) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TM_F1ENTR,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;NULL, 0xc10f1) /* в том же окне:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Return */<br>
k_(TM_F2ENTR,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;NULL, 0xc10f2) /* в новом окне: Shift+Return */<br>
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */<br>
k_(TM_LWINDOW, "Meta+Left",&nbsp;&nbsp; 0xc1012) /* перейти к левому окну&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TM_UWINDOW, "Meta+Up",&nbsp;&nbsp;&nbsp;&nbsp; 0xc1013) /* - к верхнему окну&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TM_RWINDOW, "Meta+Right",&nbsp;&nbsp;0xc1014) /* - к правому окну&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TM_DWINDOW, "Meta+Down",&nbsp;&nbsp; 0xc1015) /* - к нижнему окну&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TM_SYNCPOS, "Meta+Return", 0xc1004) /* Синхронизировать позицию:&nbsp;&nbsp;*/<br>
a_(TM_SYNCPOS, "Ctrl+Enter",&nbsp;&nbsp;0xc1004) /*&nbsp;&nbsp;&nbsp;&nbsp;открыть в соседнем окне */<br>
a_(TM_LWINDOW, "Ctrl+[4]",&nbsp;&nbsp;&nbsp;&nbsp;0xc1012) /*&nbsp;&nbsp;&nbsp;&nbsp;упомянутый здесь файл и */<br>
a_(TM_UWINDOW, "Ctrl+[8]",&nbsp;&nbsp;&nbsp;&nbsp;0xc1013) /*&nbsp;&nbsp;&nbsp;&nbsp;перейти в указанную тут */<br>
a_(TM_RWINDOW, "Ctrl+[6]",&nbsp;&nbsp;&nbsp;&nbsp;0xc1014) /*&nbsp;&nbsp;&nbsp;&nbsp;строку -- работает для: */<br>
a_(TM_DWINDOW, "Ctrl+[2]",&nbsp;&nbsp;&nbsp;&nbsp;0xc1015) /* grep, (uni)diff, compilers */<br>
/*<br>
As a consequence of First Principle, Return & Tab keys do not insert any symbol<br>
into text, but instead work as cursor movement commands (there are commands for<br>
inserting TAB into text and splitting lines at cursor, though).<br>
<br>
&nbsp;&nbsp;Tab / Shift+Tab&nbsp;&nbsp;- move cursor to next / previous TAB position<br>
&nbsp;&nbsp;Ctrl+Tab&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - cursor to last saved position, saves curent one<br>
&nbsp;&nbsp;Ctrl+Shift+Tab&nbsp;&nbsp; - insert TAB character into text - mostly for legacy make ;)<br>
&nbsp;&nbsp;Ctrl+Shift+'~'&nbsp;&nbsp; - set marker 0 (brown)<br>
&nbsp;&nbsp;Ctrl+Shift+1&nbsp;&nbsp;&nbsp;&nbsp; - set marker 1 (red)&nbsp;&nbsp; | all four mark are shown in the text<br>
&nbsp;&nbsp;Ctrl+Shift+2&nbsp;&nbsp;&nbsp;&nbsp; - set marker 2 (green) | as colored "flag" with number right<br>
&nbsp;&nbsp;Ctrl+Shift+3&nbsp;&nbsp;&nbsp;&nbsp; - set marker 3 (blue)&nbsp;&nbsp;| after the end of text in line, and<br>
&nbsp;&nbsp;Ctrl+'~'&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - go to marker 0 |&nbsp;&nbsp;&nbsp;&nbsp; | as colored dots in "position bar"<br>
&nbsp;&nbsp;Ctrl+1&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - go to marker 1 |<br>
&nbsp;&nbsp;Ctrl+2&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - go to marker 2 | all save cursor position for Ctrl+Tab<br>
&nbsp;&nbsp;Ctrl+3&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - go to marker 3 |<br>
&nbsp;&nbsp;Return&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - move cursor to beginning of next line<br>
&nbsp;&nbsp;Shift+Return&nbsp;&nbsp;&nbsp;&nbsp; - move cursor to next line, aligned with text in current one<br>
*/<br>
k_(LE_TAB,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Tab",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xd2001) /* в следующую...&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_LTAB,&nbsp;&nbsp; "Shift+Tab",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xd2202) /* в предыдущую...&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
a_(LE_LTAB,&nbsp;&nbsp; "Shift+Backtab",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xd2202) /*&nbsp;&nbsp;&nbsp;&nbsp; позицию табуляции */<br>
k_(LE_TABCHR, "Ctrl+Shift+Tab",&nbsp;&nbsp;&nbsp;&nbsp; 0xc2602) /* вставить символ TAB&nbsp;&nbsp; */<br>
a_(LE_TABCHR, "Ctrl+Shift+Backtab", 0xc2602) /*&nbsp;&nbsp;(BkTAB ≈ Shift+TAB)&nbsp;&nbsp;*/<br>
k_(TE_CMARK,&nbsp;&nbsp;"Ctrl+Tab",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xd2401) /* курсор --&gt; где были&nbsp;&nbsp; */<br>
#ifdef Q_OS_MAC&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;/* синонимы для Mac KB:&nbsp;&nbsp;*/<br>
a_(LE_TABCHR, "Alt+Shift+Backtab",&nbsp;&nbsp;0xc2602) /*&nbsp;&nbsp; вставить символ TAB */<br>
a_(TE_CMARK,&nbsp;&nbsp;"Alt+Tab",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xd2401) /*&nbsp;&nbsp; курсор --&gt; где были */<br>
#endif<br>
k_(TE_SMARK0, "Ctrl+Shift+`", 0xd2630) /* установить маркер 0, wheat */<br>
k_(TE_SMARK1, "Ctrl+Shift+1", 0xd2631) /*&nbsp;&nbsp;маркер 1 = красный флажок */<br>
k_(TE_SMARK2, "Ctrl+Shift+2", 0xd2632) /*&nbsp;&nbsp;маркер 2 = зеленый флажок */<br>
k_(TE_SMARK3, "Ctrl+Shift+3", 0xd2633) /*&nbsp;&nbsp;маркер 3 = синий флажок&nbsp;&nbsp; */<br>
a_(TE_SMARK0, "Ctrl+Shift+~", 0xd2630) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(TE_SMARK1, "Ctrl+Shift+!", 0xd2631) /* (на MacOS Х и Linux&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(TE_SMARK2, "Ctrl+Shift+@", 0xd2632) /* shift+N уже преобразовано) */<br>
a_(TE_SMARK3, "Ctrl+Shift+#", 0xd2633)<br>
k_(TE_CMARK0, "Ctrl+`",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xd2430) /* перейти к маркеру 0&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TE_CMARK1, "Ctrl+1",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xd2431) /* -&gt; 1&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TE_CMARK2, "Ctrl+2",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xd2432) /* -&gt; 2&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;NOTE: codes must */<br>
k_(TE_CMARK3, "Ctrl+3",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xd2433) /* -&gt; 3&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;be TE_S/CMARK0+N */<br>
k_(TE_CR,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Return", 0xc2004) /* в начало следующей строки&nbsp;&nbsp;*/<br>
k_(TE_RCR,&nbsp;&nbsp;&nbsp;&nbsp;"Shift+Return", 0xc2204) /* - - с выравниванием&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
/*<br>
Typing either replaces text or insert characters before cursor position (moving<br>
exising text right), depending on the current editing mode:<br>
<br>
&nbsp;&nbsp;Ctrl+Ins - set insert mode (text inserted before cursor, gradient cursor)<br>
&nbsp;&nbsp;Ctrl+Del - - replace mode (replaces existing text, indicated by solid cursor)<br>
*/<br>
k_(LE_CHAR,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; NULL, 0xf3000) /* ввести обычный символ&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_RINS, "Ctrl+Ins", 0xf3406) /* установить режим вставки */<br>
k_(LE_RREP, "Ctrl+Del", 0xf3407) /* установить режим замены&nbsp;&nbsp;*/<br>
/*<br>
Of course, any chages made to a text may be undone (and then re-applied) step-<br>
by-step - that feature was introduced in early micromirs way before it become<br>
common, so MicrMir07 still uses traditional key mapping (but also supports now<br>
standard Ctrl/Command+Z alias):<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |<br>
&nbsp;&nbsp;Ctrl+Backsp == Ctrl/Meta+Z - undo changes&nbsp;&nbsp;|&nbsp;&nbsp;Ctrl+Shift+Backsp - "slow" undo<br>
&nbsp;&nbsp;Ctrl+Return&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- redo changes&nbsp;&nbsp;|&nbsp;&nbsp;Ctrl+Shift+Return - "slow" redo<br>
*/<br>
k_(TE_UNDO,&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+Backspace",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc3403) /* откатка последней&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(TE_UNDO,&nbsp;&nbsp;&nbsp;&nbsp;"Meta+Z",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc3403) /*&nbsp;&nbsp;выполенной операции */<br>
a_(TE_UNDO,&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+Z",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc3403)<br>
k_(TE_UNUNDO,&nbsp;&nbsp;"Ctrl+Return",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc3404) /* "накатка" (работает&nbsp;&nbsp;*/<br>
k_(TE_SUNDO,&nbsp;&nbsp; "Ctrl+Shift+Backspace",&nbsp;&nbsp;0xc3603) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; после откатки) */<br>
k_(TE_SUNUNDO, "Ctrl+Shift+Return",&nbsp;&nbsp;&nbsp;&nbsp; 0xc3604)<br>
/*<br>
Although one can insert spaces into text by tapping space bar (in insert mode),<br>
that's not how it is usually done in MicroMir, since there are special commands<br>
for text modification:<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |&nbsp;&nbsp;+Shift (stronger)<br>
&nbsp;&nbsp;------------+----------------------------------+--------------------------<br>
&nbsp;&nbsp;Ins&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | insert empty character (space)&nbsp;&nbsp; | insert empty line<br>
&nbsp;&nbsp;Del&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | delete current character&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | delete current line<br>
&nbsp;&nbsp;Backspace&nbsp;&nbsp; | delete/clear char left of cursor | delete/clear word on left<br>
&nbsp;&nbsp;------------+----------------------------------+--------------------------<br>
&nbsp;&nbsp;Ctrl+D&nbsp;&nbsp;&nbsp;&nbsp; - delete from cursor to end of line<br>
&nbsp;&nbsp;Ctrl+U&nbsp;&nbsp;&nbsp;&nbsp; - delete/clear all characters up to cursor position<br>
&nbsp;&nbsp;Ctrl+W&nbsp;&nbsp;&nbsp;&nbsp; - delete word under cursor (and trailing spaces) from the text<br>
&nbsp;&nbsp;Esc,Ctrl+D - delete everything from cursor to end of file<br>
&nbsp;&nbsp;Esc,Ctrl+U - delete everything from the beginning up to cursor (very strong)<br>
*/<br>
k_(LE_IC,&nbsp;&nbsp;&nbsp;&nbsp; "Ins",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xf4006) /* вставить пробел на месте курсора&nbsp;&nbsp; */<br>
k_(LE_DC,&nbsp;&nbsp;&nbsp;&nbsp; "Del",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xf4007) /*&nbsp;&nbsp;&nbsp;&nbsp;удалить символ под курсором&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TE_IL,&nbsp;&nbsp;&nbsp;&nbsp; "Shift+Ins",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc4206) /* вставить пустую строку&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TE_DL,&nbsp;&nbsp;&nbsp;&nbsp; "Shift+Del",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xf4207) /* удалить текущую строку&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_BS,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Backspace", 0xc4003) /* удалить/очистить символ слева&nbsp;&nbsp; */<br>
k_(LE_DLWORD, "Shift+Backspace", 0xc4203) /* удалить слово влево от курсора&nbsp;&nbsp;*/<br>
k_(LE_DEOL,&nbsp;&nbsp; "Ctrl+D",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc4444) /* - конец строки за курсором&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_DBGOL,&nbsp;&nbsp;"Ctrl+U",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc4455) /* - начало строки до курсора&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_DWORD,&nbsp;&nbsp;"Ctrl+W",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc4457) /* - слово и пробелы за ним&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TE_CLRBEG, "Esc,Ctrl+U",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc4c55) /* удалить текст за курсором&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TE_CLREND, "Esc,Ctrl+D",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc4c44) /* - до курсора&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(много строк) */<br>
/*<br>
For those who are used to Return splitting text line - it can work that way,<br>
if you press Home before Return (MicroMir uses Home & Esc keyes as prefixes,<br>
both should be pressed and released before the next key (unlike Shift/Ctrl/…)<br>
<br>
&nbsp;&nbsp;Home,Return&nbsp;&nbsp;&nbsp;&nbsp;- split the line, moving right part to beginning of new line<br>
&nbsp;&nbsp;Home,Backspace - join lines back, moving text as:&nbsp;&nbsp;&nbsp;&nbsp;+------&nbsp;&nbsp;&nbsp;&nbsp; +----------<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |like█&nbsp;&nbsp;&lt;-&gt; |like█this<br>
&nbsp;&nbsp;Home,Ins - split the line vertically&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |this&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |<br>
&nbsp;&nbsp;Home,Del - join back:&nbsp;&nbsp;+---------&nbsp;&nbsp;&nbsp;&nbsp; +----------<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |like█&nbsp;&nbsp;&nbsp;&nbsp; &lt;-&gt; |like█that&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; (both join commands<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |&nbsp;&nbsp;&nbsp;&nbsp; that&nbsp;&nbsp;&nbsp;&nbsp; |&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; discard text right<br>
&nbsp;&nbsp;Meta+Shift+{arrow} or&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; of cursor position)<br>
&nbsp;&nbsp;Ctrl+Shift+Numpad{arrows}<br>
&nbsp;&nbsp;&nbsp;&nbsp;move selected text (or from cursor to end of the line)<br>
&nbsp;&nbsp;&nbsp;&nbsp;in given direction, overwriting whatever it moves over<br>
*/<br>
k_(TE_BLIN,&nbsp;&nbsp;&nbsp;&nbsp; "Home,Return",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xc5804) /* разрезать строку в начало&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TE_SLIN,&nbsp;&nbsp;&nbsp;&nbsp; "Home,Ins",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc5806) /* разрезать строку вертикально */<br>
k_(TE_NBLIN,&nbsp;&nbsp;&nbsp;&nbsp;"Home,Backspace",&nbsp;&nbsp; 0xc5803) /* склеить строки&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TE_NSLIN,&nbsp;&nbsp;&nbsp;&nbsp;"Home,Del",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc5807) /* склеить строки вертикально&nbsp;&nbsp; */<br>
k_(LE_MOVLEFT,&nbsp;&nbsp;"Meta+Shift+Left",&nbsp;&nbsp;0xf5312)<br>
a_(LE_MOVLEFT,&nbsp;&nbsp;"Ctrl+Shift+[4]",&nbsp;&nbsp; 0xf5312)<br>
k_(TE_MOVUP,&nbsp;&nbsp;&nbsp;&nbsp;"Meta+Shift+Up",&nbsp;&nbsp;&nbsp;&nbsp;0xf5313) /* перетаскивание&nbsp;&nbsp;*/<br>
a_(TE_MOVUP,&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+Shift+[8]",&nbsp;&nbsp; 0xf5313) /*&nbsp;&nbsp;текста вместе&nbsp;&nbsp;*/<br>
k_(LE_MOVRIGHT, "Meta+Shift+Right", 0xf5314) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;с кусором&nbsp;&nbsp;*/<br>
a_(LE_MOVRIGHT, "Ctrl+Shift+[6]",&nbsp;&nbsp; 0xf5314)<br>
k_(TE_MOVDOWN,&nbsp;&nbsp;"Meta+Shift+Down",&nbsp;&nbsp;0xf5315)<br>
a_(TE_MOVDOWN,&nbsp;&nbsp;"Ctrl+Shift+[2]",&nbsp;&nbsp; 0xf5315)<br>
/*<br>
Rudimental text formatting:<br>
<br>
&nbsp;&nbsp;Ctrl+T - center non-space text in current line using current window width<br>
&nbsp;&nbsp;F6 - do some formatting (depending of what pre-condition is applicable):<br>
<br>
&nbsp;&nbsp; * Split Very Long Lines (VLL): if the text in current line does not fit into<br>
&nbsp;&nbsp;&nbsp;&nbsp; window, then unfold it to several lines (splitting on word boundary unless<br>
&nbsp;&nbsp;&nbsp;&nbsp; that makes gap too big), adding continuation mark (sky blu '»') at the end<br>
<br>
&nbsp;&nbsp; * However, if current line already ends with continuation mark, do just the<br>
&nbsp;&nbsp;&nbsp;&nbsp; opposite - fold all consequent lines with that mark into single one; that<br>
&nbsp;&nbsp;&nbsp;&nbsp; line may be no fully editable, but should be Ok for saving it into a file<br>
&nbsp;&nbsp;&nbsp;&nbsp; (unless the length is more than current MicroMir limit of 4K per line)<br>
<br>
&nbsp;&nbsp; * If neither of above applies, try to join current line with the next one,<br>
&nbsp;&nbsp;&nbsp;&nbsp; provided the combined content fits into the window width<br>
*/<br>
k_(LE_CENTRX, "Ctrl+T", 0xc5454)<br>
k_(TE_FORMAT, "F6",&nbsp;&nbsp;&nbsp;&nbsp; 0xf5035)<br>
/*<br>
More formatting implemented in Lua (auto-loaded at startup, see ":/auto.lua"):<br>
<br>
&nbsp;&nbsp;^J,F6...F6,F6...&nbsp;&nbsp;- mark left/right margins by block selection, then format<br>
&nbsp;&nbsp;Ctrl+F6,F6,F6...&nbsp;&nbsp;- use previous margins&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; line-by-line<br>
&nbsp;&nbsp;Shift+F6&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- format all lines up to the next empty one<br>
<br>
Format operation tries to fill the selected area with just enough text, splits<br>
content past right edge into next line or grabs missing content from next line<br>
(no hiphenation - breaks only at spaces, commas and semicolon, multiple spaces<br>
are squeezed into one)<br>
<br>
<b>SEARH AND REPLACE.</b> MicroMir can search in text mode (case-sensitive or not), in<br>
wildcard mode (? matches any symbol and * matches any substring), or in regular<br>
expression one (see http://qt-project.org/doc/qt-4.8/QRegExp.html); the mode is<br>
selected when entering search/replace pattern.<br>
<br>
&nbsp;&nbsp;Ctrl/Meta+F = Home End - edit search/replace pattern (start search by Return)<br>
&nbsp;&nbsp;Meta[+Ctrl]+End&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- edit search pattern for 'grep' (MacOS X/Linux only)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+. (&gt;)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - insert light blue 'ʈ&gt;ʀ' (marks start of replacement)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+/&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - insert light blue 'ʈ/ʀ' (start of filelist for 'grep')<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+6 (^) - standard (text mode) search&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;‹st·--›<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+7 (&) - search by regular expression&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; ‹re·--›<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+8 (*) - search with wildcards&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;‹wc·--›<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+I&nbsp;&nbsp;&nbsp;&nbsp; - toggle case sensitivity: ignore case ‹--·ic› / not ‹--·cs›<br>
<br>
&nbsp;&nbsp;Ctrl+G&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; =&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; End - search (for previously entered pattern) forward<br>
&nbsp;&nbsp;Ctrl+Shift+G = Shift+End - search backward (up)<br>
&nbsp;&nbsp;Ctrl+B == Ctrl+&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;End - replace (if matches) or search down (if not)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+Shift+End - replace (if matches) or search up (if not)<br>
*/<br>
k_(TE_SENTR,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Home,End", 0xc6811) /* ввод подстроки поиска и замены&nbsp;&nbsp; */<br>
a_(TE_SENTR,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Meta+F", 0xc6811) /*&nbsp;&nbsp;&nbsp;&nbsp;(Return или End - поиск вниз, */<br>
a_(TE_SENTR,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+F", 0xc6811) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;+Shift - поиск вверх) */<br>
k_(LE_HCHAR0,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Ctrl+,", 0xc642c) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_HCHAR1,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Ctrl+/", 0xc642f) /* начало списка файлов (для grep)&nbsp;&nbsp;*/<br>
k_(LE_HCHAR2,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Ctrl+.", 0xc642e) /* начало подстроки замены&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LA_STDMODE,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+6", 0xc6436) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | стандартный ‹st› */<br>
k_(LA_REMODE,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Ctrl+7", 0xc6437) /* Режим поиска: | regexp&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;‹re› */<br>
k_(LA_WCMODE,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Ctrl+8", 0xc6438) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; | wildcards&nbsp;&nbsp; ‹wx› */<br>
k_(LA_IGNOREC,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+I", 0xc6449)<br>
k_(TE_SDOWN,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+G", 0xd6011)<br>
a_(TE_SDOWN,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "End", 0xd6011) /* повторение поиска вниз */<br>
k_(TE_SUP,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Shift+End", 0xd6211) /* повторить поиск вверх&nbsp;&nbsp;*/<br>
a_(TE_SUP,&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+Shift+G",&nbsp;&nbsp; 0xd6211)<br>
k_(TE_RUP,&nbsp;&nbsp;&nbsp;&nbsp;"Ctrl+Shift+End", 0xc6611) /* замена/поиск вверх */<br>
k_(TE_RDOWN,&nbsp;&nbsp;"Ctrl+End",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc6411) /* замена/поиск вниз&nbsp;&nbsp;*/<br>
a_(TE_RDOWN,&nbsp;&nbsp;"Ctrl+B",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc6411)<br>
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */<br>
k_(TE_GENTR,&nbsp;&nbsp;"Meta+Ctrl+F",&nbsp;&nbsp;&nbsp;&nbsp;0xc6511) /* глобальный поиск (grep, по всем&nbsp;&nbsp;*/<br>
a_(TE_GENTR,&nbsp;&nbsp;"Meta+Ctrl+End",&nbsp;&nbsp;0xc6511) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;файлам из списка) */<br>
a_(TE_GENTR,&nbsp;&nbsp;"Meta+End",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xc6511) /* искать&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TM_GREP,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;NULL,&nbsp;&nbsp;0xc65f1) /*&nbsp;&nbsp; в том же окне:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Return */<br>
k_(TM_GREP2,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; NULL,&nbsp;&nbsp;0xc65f2) /*&nbsp;&nbsp; в новом окне:&nbsp;&nbsp;&nbsp;&nbsp; Shift+Return */<br>
/*<br>
<b>Second MicroMir principle:</b> text is moved around using copy / paste buffer, with<br>
basic operations being "add something to the buffer" and "paste the buffer into<br>
text". Once the buffer is used, it can be pasted again, but the very next "add"<br>
command will clear it, starting saving anew. Older micromir clones used to have<br>
at least two separate buffers (for character and lines), which made intergation<br>
with system clipboard problematic, but MicroMir07 has only 1, kept in sync with<br>
clipboard by some magic that mostly works (if it does not, manual sync commands<br>
are provided for your convenience):<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;F1 - add ("save") current character (or selection) to copy/paste buffer<br>
&nbsp;&nbsp;Shift+F1 - save character (selection) and then delete it from the text<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;F2 - add ("save") word under cursor to copy/paste buffer<br>
&nbsp;&nbsp;Shift+F2 - save word and delete it<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;F3 - add ("save") current line to copy/paste buffer<br>
&nbsp;&nbsp;Shift+F3 - save line and delete it<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;F4 - paste the buffer content into text (does not clear the buffer)<br>
Note<br>
&nbsp;&nbsp;Multi-line text in c/p-buffer inserted depending on where it came from: block<br>
&nbsp;&nbsp;selection pasted alighed over existing text, for clipboard contents new empty<br>
&nbsp;&nbsp;line is inserted on each CR/LF (and text starts at left margin)<br>
*/<br>
k_(LE_CCHAR,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"F1", 0xf7030) /* добавить символ в буфер ("запомнить") */<br>
k_(LE_CDCHAR, "Shift+F1", 0xf7230) /* запомнить символ с удалением&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_CWORD,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"F2", 0xc7031) /* запомнить текущее слово&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(LE_CDWORD, "Shift+F2", 0xc7231) /* - - текущее слово с удалением&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TE_CLIN,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "F3", 0xc7032) /* добавить строку в буфер ("запомнить") */<br>
k_(TE_CDLIN,&nbsp;&nbsp;"Shift+F3", 0xc7232) /* - - запомнить строку с удалением&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_PASTE,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"F4", 0xc7033) /* вспомнить все запомненное&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
/*<br>
&nbsp;&nbsp;Ctrl/Meta+C - save selection (if any), and force copy to system clipboard<br>
&nbsp;&nbsp;Ctrl/Meta+V - paste from system clipboard<br>
&nbsp;&nbsp;&nbsp;&nbsp; Shift+F4 - mark the buffer as "used" (next save will clear it)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+F1 - "re-open" copy/paste buffer (remove "used" mark)<br>
*/<br>
k_(TE_TOCLIP,&nbsp;&nbsp; "Ctrl+C", 0xf7443)<br>
a_(TE_TOCLIP,&nbsp;&nbsp; "Meta+C", 0xf7443)<br>
k_(TE_FROMCB,&nbsp;&nbsp; "Ctrl+V", 0xc7456)<br>
a_(TE_FROMCB,&nbsp;&nbsp; "Meta+V", 0xc7456)<br>
k_(LE_CPCLOS, "Shift+F4", 0xf7233) /* начнем запоминать по новой */<br>
k_(LE_CPOPEN,&nbsp;&nbsp;"Ctrl+F1", 0xf7430) /* наоборот, будем добавлять&nbsp;&nbsp;*/<br>
/*<br>
<b>SELECTION.</b> Although text can be copied without selecting anything, MicroMir has<br>
mechanism of rectangular selections: Shift+{arrows} makes "temporary" selection<br>
(grey rectangle),&nbsp;&nbsp;F5 followed by any regular cursor movement makes "permanent"<br>
one (light blue rectangle).<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; |&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; (any movement&nbsp;&nbsp;&nbsp;&nbsp;- start/extend<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Shift+{arrows} - start/extend&nbsp;&nbsp;|&nbsp;&nbsp;F5 ..&nbsp;&nbsp;except for&nbsp;&nbsp; ..&nbsp;&nbsp; "permanent"<br>
&nbsp;&nbsp; Alt +Shift+{arrows}&nbsp;&nbsp;&nbsp;&nbsp;"temporary"&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; [Shift+]Return)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; selection<br>
&nbsp;&nbsp; Ctrl+Shift+Left/Right&nbsp;&nbsp;&nbsp;&nbsp;selection<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Typing DO NOT overwrites selected text<br>
*/<br>
a_(LE_LEFT,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Shift+Left",&nbsp;&nbsp;0xe0012) /* влево&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
a_(TE_UP,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Shift+Up",&nbsp;&nbsp;&nbsp;&nbsp;0xe0013) /* вверх&nbsp;&nbsp;| с отметкой&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(LE_RIGHT,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"Shift+Right", 0xe0014) /* вправо |&nbsp;&nbsp;&nbsp;&nbsp; блоком&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(TE_DOWN,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "Shift+Down",&nbsp;&nbsp;0xe0015) /* вниз&nbsp;&nbsp; |&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
a_(TW_UP,&nbsp;&nbsp;&nbsp;&nbsp; "Alt+Shift+Up",&nbsp;&nbsp;&nbsp;&nbsp;0xe0813) /* на несколько строк&nbsp;&nbsp;&nbsp;&nbsp; */<br>
a_(TW_DOWN,&nbsp;&nbsp; "Alt+Shift+Down",&nbsp;&nbsp;0xe0815) /*&nbsp;&nbsp;вверх/вниз с отметкой */<br>
a_(LE_NWORD,&nbsp;&nbsp;"Alt+Shift+Right", 0xe0814) /* -&gt; следующее слово&nbsp;&nbsp;&nbsp;&nbsp; */<br>
a_(LE_PWORD,&nbsp;&nbsp;"Alt+Shift+Left",&nbsp;&nbsp;0xe0812) /* -&gt; предыдущее слово&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(LE_BEG,&nbsp;&nbsp; "Ctrl+Shift+Left",&nbsp;&nbsp;0xe0412) /* в начало строки&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(LE_END,&nbsp;&nbsp; "Ctrl+Shift+Right", 0xe0414) /* в конец строки&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TX_MCBLOCK,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; "F5", 0xc8034) /* == "постоянный" блок&nbsp;&nbsp; */<br>
/*<br>
When selection (of either type) is active, some commands operate with selected<br>
text instead of current character, namely:<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;F1 (Ctrl/Meta+C) - add selected text to copy/paste buffer<br>
&nbsp;&nbsp;Shift+F1&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - save selection and delete from text<br>
&nbsp;&nbsp;Ins&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- insert column of spaces at left edge of selection<br>
&nbsp;&nbsp;Del&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- delete column at left edge of selection<br>
&nbsp;&nbsp;Shift+Del&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- clear the text covered by selection rectangle<br>
&nbsp;&nbsp;{character}&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- insert character at block position in all lines<br>
&nbsp;&nbsp;(last command works only when block width is 1 column, "tall cursor" mode)<br>
<br>
The following commands also can work with selections, but only with single-line<br>
ones (without selection, current character is converted and cursor moves right)<br>
<br>
&nbsp;&nbsp;Meta+Alt+Up&nbsp;&nbsp; - convert current character (or selected block) to uppercase<br>
&nbsp;&nbsp;Meta+Alt+Down - convert current character (or selected block) to lowercase<br>
&nbsp;&nbsp;Meta+Alt+B&nbsp;&nbsp;&nbsp;&nbsp;- add / clear "bold" attribute<br>
<br>
NOTE: "bold" attribute is marked by Unicode chars U+281 / U+280 (which are very<br>
unlikely to appear in regular text) and not compatible with anything.. not even<br>
with build-in MicroMir search engine. Use at your own risk, you've been warned!<br>
*/<br>
k_(LE_CCUP,&nbsp;&nbsp;"Meta+Alt+Up",&nbsp;&nbsp; 0xf8913) /* буква -&gt; прописная */<br>
k_(LE_CCDWN, "Meta+Alt+Down", 0xf8915) /* буква -&gt; строчная&nbsp;&nbsp;*/<br>
k_(LE_CBOLD, "Meta+Alt+B",&nbsp;&nbsp;&nbsp;&nbsp;0xf8442) /* сделать жирным&nbsp;&nbsp;&nbsp;&nbsp; */<br>
/*<br>
<b>Third MicroMir principle:</b> instead of relying on sorting capabilites provided by<br>
file browser, one can keep files sorted his/her way (and also add any comments)<br>
by listing filenames in 'micros.dir'.&nbsp;&nbsp;This is a regular text file, except that<br>
it has '|' symbols in all lines in 64th position (and, optionally, in 69th one)<br>
<br>
&nbsp;&nbsp;Esc,Right&nbsp;&nbsp; - move right to the filename field (return back by Return)<br>
&nbsp;&nbsp;Esc,Down&nbsp;&nbsp;&nbsp;&nbsp;- down into file (filename taken from the text, see below)<br>
&nbsp;&nbsp;Esc,Up&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- up one level = normal exit (with changes automatically saved)<br>
&nbsp;&nbsp;Esc,PgUp&nbsp;&nbsp;&nbsp;&nbsp;- up all levels (closing the window), with changes saved<br>
&nbsp;&nbsp;Esc,Ctrl+A&nbsp;&nbsp;- abandon all changes and leave the file (up one level)<br>
&nbsp;&nbsp;Esc,Ctrl+B&nbsp;&nbsp;- exit the file, save both original and new versions<br>
&nbsp;&nbsp;Ctrl/Meta+R - reload the file (or directory contents) from disk<br>
&nbsp;&nbsp;Ctrl/Meta+S - save all changes without leaving the file<br>
*/<br>
k_(TM_INFILE, "Esc,Right",&nbsp;&nbsp;0xc9414) /* перейти к имени файла */<br>
k_(TM_FNEW,&nbsp;&nbsp; "Esc,Down",&nbsp;&nbsp; 0xf9415) /* войти в новый файл&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TM_EXIT,&nbsp;&nbsp; "Esc,Up",&nbsp;&nbsp;&nbsp;&nbsp; 0xc9413) /* выйти с сохранением&nbsp;&nbsp; */<br>
k_(TM_QUIT,&nbsp;&nbsp; "Esc,PgUp",&nbsp;&nbsp; 0xc9416) /* выйти совсем&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TM_EXOLD,&nbsp;&nbsp;"Esc,Ctrl+A", 0xc9c41) /* выйти без сохранения&nbsp;&nbsp;*/<br>
k_(TM_EXBACK, "Esc,Ctrl+B", 0xc9c42) /* - сохранив обе версии */<br>
k_(TM_RELOAD, "Meta+R",&nbsp;&nbsp;&nbsp;&nbsp; 0xc9452)<br>
a_(TM_RELOAD, "Ctrl+R",&nbsp;&nbsp;&nbsp;&nbsp; 0xc9452) /* перезагрузить файл&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TM_UPDATE, "Meta+S",&nbsp;&nbsp;&nbsp;&nbsp; 0xc9453) /* сохранить изменения&nbsp;&nbsp; */<br>
a_(TM_UPDATE, "Ctrl+S",&nbsp;&nbsp;&nbsp;&nbsp; 0xc9453)<br>
/*<br>
&nbsp;&nbsp;This "down into file" command works not only in 'micros.dir', but also in any<br>
text that have name of other file included: just move cursor to that name, then<br>
press Esc &lt;down&gt; (the name should be separated from surrounding text by spaces,<br>
commas or quotes - if is't not, you'll need to use block selection to mark it)<br>
<br>
Special <b>FILES</b> (MacOS X/Linux only):<br>
<br>
&nbsp;&nbsp;filename:git:rev == filename::rev - file revision from given git commit<br>
&nbsp;&nbsp;filename:gitlog[options]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- list of revisions, from 'git log'<br>
&nbsp;&nbsp;filename:blame[options]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - results of 'git blame' for given file<br>
&nbsp;&nbsp;filename:hg:rev&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - file revision (-r) from Mercurial commit<br>
&nbsp;&nbsp;filename:hglog[options]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - list of revisions, from 'hg log'<br>
&nbsp;&nbsp;filename:annotate[options]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- results of 'hg annotate' for given file<br>
&nbsp;&nbsp;«shell-command»&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - the result of specific shell command<br>
&nbsp;&nbsp;(double angle quotations mark can be entered by Alt+'\' and Alt+Shift+'\';<br>
&nbsp;&nbsp; when starting MicroMir in command prompt, these marks may be replaced with<br>
&nbsp;&nbsp; single quote before the parameter: ʈ&gt;ʀ mim \''git log' -- will open git log)<br>
<br>
MicroMir automatically detects read-only files and don't allow changing them...<br>
unless you insist: edit/view mode may be changed at any time (at your own risk)<br>
<br>
&nbsp;&nbsp;Ctrl+Shift+Ins - set text edit mode (green or blue cursor, can change text)<br>
&nbsp;&nbsp;Ctrl+Shift+Del - set text view mode (no changes allowed, red cursor)<br>
*/<br>
k_(TE_SRW, "Ctrl+Shift+Ins", 0xca506) /* установить режим редактирования&nbsp;&nbsp; */<br>
k_(TE_SRO, "Ctrl+Shift+Del", 0xca507) /* установить режим просмотра текста */<br>
/*<br>
<b>REPETITION:</b> Esc,N - enter repetition counter (or argument) for the next command<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;where N is either ddd decimal number like&nbsp;&nbsp; 126<br>
For example:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0ddd octal number such as 0176<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; xddd hexadecimal number&nbsp;&nbsp;&nbsp;&nbsp;x7e<br>
&nbsp;&nbsp;Esc,3,Shift+Del - delete 3 lines<br>
&nbsp;&nbsp;Esc,5,*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - same as typing ***** (insert/replace depending on the mode)<br>
&nbsp;&nbsp;Esc,N,Alt+Ins&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- insert character with given Unicode number<br>
&nbsp;&nbsp;Esc,N,Meta+Alt+'&lt;' - convert word/selection from radix-N (hex) to decimal<br>
&nbsp;&nbsp;Esc,N,Meta+Alt+'&gt;' - convert word/selection to radix-N (where N = 2..36)<br>
&nbsp;&nbsp;Esc,N,Ctrl+Shift+] - set the width (of editable part) of the text<br>
&nbsp;&nbsp;Esc,N,Ctrl+E&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - move cursor to Nth line in text<br>
&nbsp;&nbsp;Esc,N,Ctrl+H&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - move cursor to Nth position in current line<br>
<br>
Note: without repetition prefix, the last two commands work differently, which<br>
&nbsp;&nbsp;&nbsp;&nbsp; is a special convenience feature to allow easy centering specific position<br>
<br>
&nbsp;&nbsp;Ctrl+E - centers current line in the windows (vertically only)<br>
&nbsp;&nbsp;Ctrl+H - centers curret position in the windows (horizontally)<br>
*/<br>
k_(TK_PREFIX, "Home",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xcc00a) /* Home (generic) префикс&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TK_ESC,&nbsp;&nbsp;&nbsp;&nbsp;"Esc",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xcc00e) /* Esc префикс (повторитель и пр.) */<br>
k_(TK_CtrJ,&nbsp;&nbsp; "Ctrl+J",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xcc010) /* ^J префикс (используется в Lua) */<br>
k_(LE_SPCHAR, "Alt+Ins",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0xcb606) /* ввести символ по Unicode коду&nbsp;&nbsp; */<br>
k_(LE_CWDEC,&nbsp;&nbsp;"Meta+Alt+,",&nbsp;&nbsp; 0xfb92c) /* <b>&lt;</b>&nbsp;&nbsp;перевести число в десятичное */<br>
k_(LE_CWHEX,&nbsp;&nbsp;"Meta+Alt+.",&nbsp;&nbsp; 0xfb92e) /* <b>&gt;</b>&nbsp;&nbsp;- в 16-ричное / основание N&nbsp;&nbsp;*/<br>
k_(TE_SWIDTH, "Ctrl+Shift+}", 0xcb65d) /* <b>]</b>&nbsp;&nbsp;установить ширину текста&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TE_CENTR,&nbsp;&nbsp;"Ctrl+E",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xfb445) /* в середину окна по вертикали&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(LE_CENTR,&nbsp;&nbsp;"Ctrl+H",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xfb448) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; - по горизонтали&nbsp;&nbsp;*/<br>
/*<br>
<b>Lua SCRIPTING:</b> MicroMir embeds Lua scripting language (see http://www.lua.org),<br>
scripts are supposed to be opened just as regular texts and then executed using<br>
these commands (plus ":/auto.lua" automatically loaded/executed at startup):<br>
<br>
&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+J,Ctrl+J - load and executes the entire current text as Lua script<br>
&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+Shift+J&nbsp;&nbsp;- load/execute current line as (single-line) Lua script<br>
&nbsp;&nbsp;&nbsp;&nbsp;Ctrl+J,N&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;- create new unnamed/throw-away text for new Lua script<br>
*/<br>
k_(TM_LUAF,&nbsp;&nbsp;&nbsp;&nbsp;"^J,Ctrl+J", 0xfc000) /* загрузить и выполнить программу на&nbsp;&nbsp; */<br>
k_(TM_LUAS, "Ctrl+Shift+J", 0xfcc00) /*&nbsp;&nbsp;языке Lua из текущего текста/строки */<br>
k_(TM_LUAN, "^J,N",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xfc00e)<br>
/*<br>
MicroMir-specific Lua objects and operations<br>
<br>
&nbsp;&nbsp;Mk["keysequence"] = Mk["other-sequence"] -- remap existing MicroMir command<br>
&nbsp;&nbsp;Mk["keysequence"] = "text-and-commands"&nbsp;&nbsp;-- map text with embedded commands…<br>
&nbsp;&nbsp;Mk["keysequence"] = function(Tx,count)&nbsp;&nbsp; -- map Lua function to key sequence<br>
&nbsp;&nbsp;&nbsp;&nbsp;...<br>
&nbsp;&nbsp;end<br>
&nbsp;&nbsp;Mk:Do( Mk["keysequence"] ) -- execute given command (or text with commands),<br>
&nbsp;&nbsp;Mk:Do("text-and-commands") --&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;as if they were entered from the keyboard<br>
<br>
&nbsp;&nbsp;Tx = Txt.open("filename") -- create/open given file in new window<br>
&nbsp;&nbsp;Tx = Txt.open(true)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; -- open new window with unnamed file (saveable)<br>
&nbsp;&nbsp;Tx = Txt.open(false)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-- open new window with throw-away text<br>
&nbsp;&nbsp;Tx = Txt.this&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; -- reference to "this" text (may be used only from<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;text with script iself, not from functions)<br>
&nbsp;&nbsp;Tx.id -- text ID = index in Txt<br>
&nbsp;&nbsp;(should be used for safe reference, Txt[Tx.id] -&gt; Tx or nil if text replaced)<br>
<br>
&nbsp;&nbsp;Tx:focus() -- focus last opened window, associated with the text<br>
&nbsp;&nbsp;Tx:line(N) -- content of Nth line in given text (= nil if past end-of-text)<br>
&nbsp;&nbsp;Tx:lines() -- iterator over the text:&nbsp;&nbsp;for N,line in Tx:lines() do ... end<br>
&nbsp;&nbsp;Tx.X, Tx.Y -- current cursor position (assigning new value will move cursor)<br>
&nbsp;&nbsp;Tx.reX,reY -- another corner of the selection rectangle (nil if no selection)<br>
&nbsp;&nbsp;Tx.maxY&nbsp;&nbsp;&nbsp;&nbsp;-- max value of Y in text&nbsp;&nbsp;(= total number of lines in given text)<br>
&nbsp;&nbsp;Tx:go(dy)<br>
&nbsp;&nbsp;Tx:go(dx,dy) = convenience methods to move cursor around (because Lua does<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; not allow 'Tx.Y++' or 'Tx.Y += k')<br>
NOTE:<br>
&nbsp;&nbsp;attempt to move cursor out of text boundaries does not generate any error,<br>
&nbsp;&nbsp;cursor just moves as far as possible in requested direction<br>
<br>
&nbsp;&nbsp;Tx:IC("text") -- insert text into current line | moves cursor past<br>
&nbsp;&nbsp;Tx:IL("line") -- insert given line at cursor&nbsp;&nbsp; |&nbsp;&nbsp;&nbsp;&nbsp; inserted text<br>
&nbsp;&nbsp;Tx:DC(N) -- delete N characters at cursor<br>
&nbsp;&nbsp;Tx:DL(N) -- delete N lines at cursor (default = 1)<br>
&nbsp;&nbsp;Tx:gtl() -- get-text-left convenience method = gets first chunk of non-spaces<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; from text to the left of cursor position<br>
[line 525] xref: new.lua<br>
<br>
&nbsp;&nbsp;re = Re("..regex..",true/false) -- regular expression (arg: case sensitivity,<br>
&nbsp;&nbsp;re = Re[[..regex..]]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;default = case-sensitive)<br>
&nbsp;&nbsp;re:ifind("str"[,pos]) -- returns index of first match (nil if does not match)<br>
&nbsp;&nbsp;re:cap(N)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; -- returns Nth capture (only when called after 'ifind')<br>
&nbsp;&nbsp;re:caps()&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; -- all captures (to use in multi-value assignement)<br>
&nbsp;&nbsp;re:grepl("str","to")&nbsp;&nbsp;-- globally replace pattern (where \N = Nth capture)<br>
<br>
Examples (auto-loaded at startup):<br>
<br>
&nbsp;&nbsp;Mk["Ctrl+Shift+T"] = function(Tx) Tx:IC(os.date()) end<br>
&nbsp;&nbsp;Mk["Ctrl+Shift++"] = function(Tx)<br>
&nbsp;&nbsp;&nbsp;&nbsp;local result = loadstring("local X="..Tx:gtl()..";return X")()<br>
&nbsp;&nbsp;&nbsp;&nbsp;Tx:IC("= "..tostring(result))<br>
&nbsp;&nbsp;end<br>
&nbsp;&nbsp;for _,n in pairs{"F7","F8","F9"} do -- inserts current value of Fn macro into<br>
&nbsp;&nbsp;&nbsp;&nbsp;Mk["^J,"..n] = function(Tx)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; --&nbsp;&nbsp;current text (to edit and re-execute)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;local pref,Fn = "Mk."..n.."=",Mk[n]<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;if type(Fn) == 'string' then Tx:IL(pref..'"'..Fn..'"')<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;else Tx:IL(pref..'none') end<br>
&nbsp;&nbsp;&nbsp;&nbsp;end<br>
&nbsp;&nbsp;end<br>
&nbsp;&nbsp;Mk2html(Txt.this) -- convert this text to HTML (use Ctrl+Shift+J here)<br>
*/<br>
k_(TK_SM0,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; NULL, 0xfd235) /* <b>MACRO:</b> In addition to universal scripts, */<br>
k_(TK_SM1, "Shift+F7", 0xfd236) /* MikroMir provides quick method to record */<br>
k_(TK_SM2, "Shift+F8", 0xfd237) /* simple sequence of action & play it back */<br>
k_(TK_SM3, "Shift+F9", 0xfd238) /* later by just one keypress:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TK_EM0,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; NULL, 0xfd035) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
k_(TK_EM1, "F7",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xfd036) /*&nbsp;&nbsp; Shift+F7/F8/F9 - start recording macro */<br>
k_(TK_EM2, "F8",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xfd037) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; F7/F8/F9 - finish recording, or&nbsp;&nbsp;*/<br>
k_(TK_EM3, "F9",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xfd038) /*&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; execute previously recorded macro&nbsp;&nbsp;*/<br>
/*<br>
<b>SYNTAX CHECKER:</b> Automatically does minimal syntax check (mostly bracket balance<br>
and proper quoting) and highlights keywords and comments, depending on language<br>
used in the text (which is determined by file extension and/or #! in 1st line).<br>
 Currently works more or less reliable for C/C++ formatted with "proper" style,<br>
so a couple of commands added to turn it on/off dynamically (on per-file basis)<br>
<br>
&nbsp;&nbsp;Ctrl+Shift+0/)&nbsp;&nbsp;&nbsp;&nbsp;- disable syntax check / coloring for given file<br>
&nbsp;&nbsp;Ctrl+Shift+9/(&nbsp;&nbsp;&nbsp;&nbsp;- (re-)enable syntax check<br>
&nbsp;&nbsp;Ctrl+O (letter O) - toggle "show all brackets not closed in the line" mode<br>
&nbsp;&nbsp;(sequence Esc,0,Ctrl+O turns off bracket position check, reducing warnings)<br>
<br>
Note for Windows 7: keyboard shortcut Ctrl+Shift+0 does not work with default<br>
&nbsp;&nbsp; configuration. Refer to http://support.microsoft.com/kb/967893 for the fix.<br>
*/<br>
k_(TE_BRAK0,&nbsp;&nbsp;"Ctrl+Shift+0", 0xce630) /* выключить раскраску */<br>
a_(TE_BRAK0,&nbsp;&nbsp;"Ctrl+Shift+)", 0xce630)<br>
k_(TE_BRAK9,&nbsp;&nbsp;"Ctrl+Shift+9", 0xce639) /* включить раскраску&nbsp;&nbsp;*/<br>
a_(TE_BRAK9,&nbsp;&nbsp;"Ctrl+Shift+(", 0xce639)<br>
k_(TE_SHBRAK, "Ctrl+O",&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0xce44f) /* переключить режим&nbsp;&nbsp; */<br>
/*<br>
<b>MISCELLANEOUS.</b><br>
<br>
&nbsp;&nbsp;Ctrl+A - abandon (cancel) current command (e.g. stop entering search pattern)<br>
&nbsp;&nbsp;Ctrl+K - display unicode value of the current character in the info window<br>
&nbsp;&nbsp;Ctrl+L - display line number in the info window (default)<br>
&nbsp;&nbsp;Ctrl+Q - ask for shell command and execute it (MacOS X/Linux only)<br>
&nbsp;&nbsp; in shell command:<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;light blue 'ʈ•ʀ' (Ctrl+'&lt;') is replaced with the name of current file,<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;light blue 'ʈ/ʀ' (Ctrl+'/') replaced with full path to current file<br>
*/<br>
k_(TK_NONE,&nbsp;&nbsp; NULL,&nbsp;&nbsp;&nbsp;&nbsp; 0xce000) /* None command */<br>
k_(TK_BREAK,&nbsp;&nbsp;"Ctrl+A", 0xce441)<br>
k_(TK_CHARK,&nbsp;&nbsp;"Ctrl+K", 0xce44b)<br>
k_(TK_LINFO,&nbsp;&nbsp;"Ctrl+L", 0xce44c)<br>
k_(TM_SHELL,&nbsp;&nbsp;"Ctrl+Q", 0xce451) /* ввести и выполнить команду shell&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TM_FEXEC,&nbsp;&nbsp;NULL,&nbsp;&nbsp;&nbsp;&nbsp; 0xceef1) /* в том же окне:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Return */<br>
k_(TM_F2EXEC, NULL,&nbsp;&nbsp;&nbsp;&nbsp; 0xceef2) /* в новом окне:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Shift+Return */<br>
/*<br>
<b>GRADIENTS:</b> By default MicroMir07 fills background with linear gradient (because<br>
solid background is just too boring), which may be configured using Preferences<br>
dialog. Full format of gradient specification is the following (square brackets<br>
mark optional parts):<br>
<br>
&nbsp;&nbsp;primColor[/offColor][,start-stop][;tilt][,2nd:prim2][,3rd:prim3][,4th:prim4]<br>
<br>
where primColor - primary gradient color for first (default) color palette<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; offColor - off-color (default "white")<br>
&nbsp;&nbsp;&nbsp;&nbsp; start/stop - start/stop points from 0.0 (top left) to 1.0 (bottom right)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; tilt - from 0 (horizontal) to 1 (diagonal) to 4 (even more tilted)<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;primN - primary color for Nth color palette<br>
Color:<br>
&nbsp;&nbsp;#rrggbb 24-bit hexadecimal RGB value&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;for example:&nbsp;&nbsp;&nbsp;&nbsp;#ffefd5<br>
&nbsp;&nbsp;SVG color names (see http://www.w3.org/TR/SVG/types.html) &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;papayawhip<br>
&nbsp;&nbsp;Hue degrees (in that case Sat=16% and Val=100% is forced)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;40° or 40*<br>
<br>
&nbsp;&nbsp;Ctrl+Alt+1 - switch to first / default palette in current window<br>
&nbsp;&nbsp;Ctrl+Alt+2 - switch to palette 2 (if not confiured, using 80° light green)<br>
&nbsp;&nbsp;Ctrl+Alt+3 - switch to palette 3 (default is 200° sky blue)<br>
&nbsp;&nbsp;Ctrl+Alt+4 - switch to palette 4 (default is "mistyrose")<br>
*/<br>
k_(TW_GRAD1, "Ctrl+Alt+1", 0xcfc31) /* включить палитру 1 в текущем окне */<br>
k_(TW_GRAD2, "Ctrl+Alt+2", 0xcfc32) /* - палитру 2 (зеленая)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TW_GRAD3, "Ctrl+Alt+3", 0xcfc33) /* - палитру 3 (голубая)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
k_(TW_GRAD4, "Ctrl+Alt+4", 0xcfc34) /* - палитру 4 (розовая)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
/*<br>
<b>KEY MAPPING</b>. In order to accommodate different keyboards, MicroMir provides key<br>
aliases that may be specified in the Preferences dialog as comma-separated list<br>
of "alias=key" pairs. Syntax for both elements is consistent with this document<br>
(if you are not sure what is the name of particular key,&nbsp;&nbsp;try starting MicroMir<br>
with '-kb' option, the names of keys will be displayed in the Info window), and<br>
the alias automatically works with any modifier. For example, if keyboard lacks<br>
Ins/Del keys (but have extra function or numpad keys), then you can use these:<br>
<br>
&nbsp;&nbsp;F11=Ins,F12=Del&nbsp;&nbsp; -- use F11 as alias for Ins and F12 as alias for Del<br>
&nbsp;&nbsp;[0]=Ins,[.]=Del&nbsp;&nbsp; -- use numpad keys for Ins and Del<br>
<br>
See "Lua SCRIPTING" for remapping MicroMir commands to different key sequences.<br>
-------------------------------------------------------------------------------<br>
Some useful Alt+CHARs (most already present on Mac, but a couple of them moved)<br>
*/<br>
a_(0, "Alt+1", 0x00B9) /* ¹ */<br>
a_(0, "Alt+2", 0x00B2) /* ² */&nbsp;&nbsp;&nbsp;&nbsp;a_(0, "Alt+Shift+@", 0x2122) /* ™ (moved) */<br>
a_(0, "Alt+3", 0x00B3) /* ³ */&nbsp;&nbsp;&nbsp;&nbsp;a_(0, "Alt+Shift+:", 0x205D) /* ⁝&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
a_(0, "Alt+9", 0x2023) /* ‣ */<br>
a_(0, "Alt+=", 0x2248) /* ≈ (moved)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; W↑&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;*/<br>
a_(0, "Alt+X", 0x00D7) /* ×&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;A← S↓ D→&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; */<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a_(0, "Alt+W", 0x2191)<br>
a_(0, "Alt+A", 0x2190)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a_(0, "Alt+S", 0x2193)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a_(0, "Alt+D", 0x2192)<br>
#ifndef Q_OS_MAC<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a_(0, "Alt+Shift+#", 0x2039) /* ‹ (3) */<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a_(0, "Alt+Shift+$", 0x203A) /* › (4) */<br>
a_(0, "Alt+6",&nbsp;&nbsp;0x00A7) /* § */&nbsp;&nbsp; a_(0, "Alt+Shift+&", 0x2021) /* ‡ (7) */<br>
a_(0, "Alt+8",&nbsp;&nbsp;0x2022) /* • */&nbsp;&nbsp; a_(0, "Alt+Shift+*", 0x00B0) /* ° (8) */<br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;a_(0, "Alt+Shift+(", 0x00B7) /* · (9) */<br>
a_(0, "Alt+-",&nbsp;&nbsp;0x2013) /* – */&nbsp;&nbsp; a_(0, "Alt+Shift++", 0x00B1) /* ±&nbsp;&nbsp;&nbsp;&nbsp; */<br>
a_(0, "Alt+R",&nbsp;&nbsp;0x00AE) /* ® */<br>
a_(0, "Alt+T",&nbsp;&nbsp;0x2020) /* † */<br>
a_(0, "Alt+O",&nbsp;&nbsp;0x00F8) /* ø */&nbsp;&nbsp; a_(0, "Alt+Shift+O", 0x00D8) /* Ø */<br>
a_(0, "Alt+[",&nbsp;&nbsp;0x201C) /* “ */&nbsp;&nbsp; a_(0, "Alt+Shift+{", 0x201D) /* ” */<br>
a_(0, "Alt+]",&nbsp;&nbsp;0x2018) /* ‘ */&nbsp;&nbsp; a_(0, "Alt+Shift+}", 0x2019) /* ’ */<br>
a_(0, "Alt+\\", 0x00AB) /* « */&nbsp;&nbsp; a_(0, "Alt+Shift+|", 0x00BB) /* » */<br>
a_(0, "Alt+G",&nbsp;&nbsp;0x00A9) /* © */<br>
a_(0, "Alt+J",&nbsp;&nbsp;0x2206) /* ∆ */<br>
a_(0, "Alt+;",&nbsp;&nbsp;0x2026) /* … */&nbsp;&nbsp; a_(0, "Alt+Shift+V", 0x25CA) /* ◊ */<br>
a_(0, "Alt+M",&nbsp;&nbsp;0x00B5) /* µ */<br>
a_(0, "Alt+,",&nbsp;&nbsp;0x2264) /* ≤ */<br>
a_(0, "Alt+.",&nbsp;&nbsp;0x2265) /* ≥ */<br>
a_(0, "Alt+/",&nbsp;&nbsp;0x00F7) /* ÷ */&nbsp;&nbsp; a_(0, "Alt+Shift+?", 0x00BF) /* ¿ */<br>
#endif<br>
k_(TK_ZERO,0,0)<br>
k_END_OF_TABLE<br>
</font></span>
</div>

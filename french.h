#ifndef _LANG
#define _LANG
#define MSG_SHIFT L"Maj"
#define MSG_CTRL L"Ctrl"
#define MSG_ALT L"Alt"
#define MSG_OK L"&OK"
#define MSG_CANCEL L"A&nnuler"
#define MSG_CLOSE L"&Fermer"
#define MSG_ERROR L"Erreur"
#define MSG_INFO L"Information"
#define MSG_WARNING L"Attention!"
#define MSG_QUESTION L"Question"
#define MSG_INPUTDEFTITLE "Demande d'information"
#define MSG_H_OUTPUT "Sortie"
#define MSG_LUADB_TITLE L"Console lua"
#define MSG_CLEAR L"&Vider"
#define MSG_EVAL L"&Evaluer"
#define MSG_EVALLBL L"Evaluer: "
#define MSG_WINDOWTITLE L"%ls - %ls"
#define MSG_UNTITLED L"Sans titre %d"
#define MSG_SB1 L"Li %d, col %d, %d%%"
#define MSG_SB2 L"Li %d, col %d \xE0 li %d, col%d, %d%%"
#define MSG_SAVECHANGES L"Voulez-vous enregistrer les modifications apport\xE9" L"es \xE0 %s avant de quitter ?"
#define MSG_RELOAD_WARNING L"\xCAtes-vous s\xFBr de vouloir recharger le fichier actuellement ouvert ? \r\nVous allez perdre toutes les modifications effectu\xE9" L"es depuis son dernier enregistrement."
#define MSG_CROSSMODIFICATIONS L"Le fichier %s a \xE9t\xE9 modifi\xE9 par un autre processus. Voulez-vous le recharger ?\r\nVous risquez de perdre un travail non sauvegard\xE9."
#define MSG_SAVEONCRASH L"Une erreur fatale est survenue et l'application doit \xEAtre imm\xE9" L"diatement ferm\xE9" L"e.\r\nUne sauvegarde du fichier en cours d'\xE9" L"dition a \xE9t\xE9 enregistr\xE9" L"e dans le fichier %ls-auto-save.tmp.\r\nVeuillez nous excuser pour le d\xE9sagr\xE9ment encouru."
#define MSG_GOTOLINETITLE L"Atteindre la ligne"
#define MSG_GOTOLINETEXT L"Atteindre la &ligne (max. %d : )"
#define MSG_GOTOLINE_RANGE_ERROR L"Num\xE9ro de ligne hors limites."
#define MSG_REPL_SEARCH L"Re&chercher : "
#define MSG_REPL_REPLACE L"Re&mplacer par : "
#define MSG_REPL_CBCASE L"&Diff\xE9rencier la casse"
#define MSG_REPL_CBLUA L"E&xpression r\xE9guli\xE8re"
#define MSG_REPL_CBUTF8 L"Extensions &unicode"
#define MSG_REPL_DIRECTION L"Direction de recherche: "
#define MSG_REPL_DIRECTION_UP L"&Haut"
#define MSG_REPL_DIRECTION_DOWN L"&Bas"
#define MSG_REPL_DLGTITLE1 L"Rechercher"
#define MSG_REPL_DLGTITLE2 L"Rechercher et remplacer"
#define MSG_REPL_BTNOK1 L"&Rechercher"
#define MSG_REPL_BTNOK2 L"&Remplacer tout"
#define MSG_REPL_REGEXERROR L"Erreur de syntaxe dans l'expression r\xE9guli\xE8re"
#define MSG_REPL_REPLSUCCESS L"%d remplacements effectu\xE9s"
#define MSG_OPENTITLE L"Ouvrir"
#define MSG_SAVETITLE L"Enregistrer sous"
#define MSG_FILENAMEFILTER L"Tous les fichiers\0*.*\0"
#define MSG_READ_ERROR L"Impossible d'ouvrir le fichier."
#define MSG_WRITE_ERROR L"Impossible d'enregistrer le fichier."

#define MSG_FILEMENU L"&Fichier"
#define MSG_EDITMENU L"&Edition"
#define MSG_FORMATMENU L"Forma&t"
#define MSG_LINEENDINGMENU L"Sauts de &ligne"
#define MSG_ENCODINGMENU "&Encodage"
#define MSG_INDENTMENU L"&Mode d'indentation"
#define MSG_LINEWRAP L"&Retour automatique \xE0 la ligne"
#define MSG_AUTORELOAD L"Rec&hargement automatique"
#define MSG_NEW L"&Nouveau\aCtrl+N"
#define MSG_OPEN L"&Ouvrir...\aCtrl+O"
#define MSG_OPEN_NEW_INSTANCE L"Ouvrir dans une nouvelle &instance...\aCtrl+Maj+O"
#define MSG_OPEN_FORCE L"Ouvrir en &utilisant l'encodage s\xE9lectionn\xE9..."
#define MSG_SAVE L"Enre&gistrer\aCtrl+S"
#define MSG_SAVE_AS L"Enregistrer &sous...\aCtrl+Maj+S"
#define MSG_RECENT_FILES L"&Fichiers R\xE9" L"cents"
#define MSG_EXIT L"&Quitter\aAlt+F4"
#define MSG_CLOSE2 L"&Fermer\aCtrl+F4"
#define MSG_UNDO L"&Annuler\aCtrl+Z"
#define MSG_REDO L"R\xE9ta&blir\aCtrl+Maj+Z"
#define MSG_COPY L"&Copier\aCtrl+C"
#define MSG_CUT L"Co&uper\aCtrl+X"
#define MSG_PASTE L"Co&ller\aCtrl+V"
#define MSG_SELECTALL L"S\xE9lectionner &tout\aCtrl+A"
#define MSG_GOTOLINE L"Atteindre la li&gne...\aCtrl+G"
#define MSG_FIND L"Re&chercher...\aCtrl+F"
#define MSG_REPLACE L"&Rechercher et remplacer...\aCtrl+H"
#define MSG_FINDNEXT L"Rechercher le sui&vant\aF3"
#define MSG_FINDPREV "Rechercher le &pr\xE9" L"c\xE9" L"dent\aMaj+F3"
#define MSG_SWITCHCURSOR L"Changer de curs&eur\aF4"
#define MSG_JOINCURSOR L"&Joindre les curseurs\aMaj+F4"
#define MSG_CURSORSELECT L"S\xE9lectionner jus&qu'au second curseur\aCtrl+Maj+A"
#define MSG_INDENT_TAB L"&Tabulations"
#define MSG_INDENT_SPACES L"%d &espaces"
#define MSG_INDENT_SPACES_DEF L"&Espaces"
#define MSG_LE_DOS L"&DOS/Windows (CRLF)"
#define MSG_LE_UNIX L"&Unix/linux (LF)"
#define MSG_LE_MAC L"&Macintosh (CR)"
#define MSG_ENC0 L"Windows/&ANSI"
#define MSG_ENC1 L"UTF-&7"
#define MSG_ENC2 L"UTF-&8"
#define MSG_ENC3 L"UTF-8 avec &BOM"
#define MSG_ENC4 L"&Unicode"
#define MSG_ENC5 L"Unicode avec B&OM"
#define MSG_ENC6 L"Unicode bi&g-endian"
#define MSG_ENC7 L"Unicode big-endian a&vec BOM"
#define MSG_ENC8 L"ISO-8859-1&5/Latin-9"
#define MSG_ENC9 L"IBM &437 / International DOS"
#define MSG_ENC10 L"IBM 850 / Latin-1 &DOS"
#define MSG_ENC11 L"IBM &EBCDIC"
#define MSG_ENC12 L"&Macintosh latin-1"
#define MSG_RELOAD L"Re&charger"
#define MSG_RELOAD_FORCEENCODING L"Recharger en utilisant l'&encodage s\xE9lectionn\xE9\aCtrl+F5"
#define MSG_SMARTHOME L"Touche &origine intelligente"
#define MSG_KEEPINDENT L"Suivre l'&indentation"
#define MSG_SHOWSTATUS L"AFficher la barre d'&\xE9tat"
#define MSG_TOOLS_MENU L"&Outils"
#define MSG_CONSOLE L"&Console lua"
#define MSG_HELPMENU L"Ai&de"
#define MSG_ABOUT L"&\xC0 propos de 6Pad..."
#define MSG_ABOUTTITLE L"\xC0 propos de  6Pad"
#define MSG_ABOUTMSG L"6Pad\r\n\tVersion " VERSION_STRING L"\r\n\xA9 2010-2012, QuentinC http://quentinc.net/"
#endif


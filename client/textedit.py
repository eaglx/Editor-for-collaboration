import sys
from PyQt4 import QtGui, QtCore
import re
import os
import subprocess

class Window(QtGui.QMainWindow):
    def __init__(self):
        super(Window, self).__init__()
        self.setGeometry(150, 150, 700, 600)
        self.setWindowTitle("TextEdit")
        self.fixSizeString = False

        self.colorsUsers = ["#ff0000","ffd700","ff7200","b89300","#07f000","006d00","00fecc","0061cc","9c61cc", "ff43d4"]
        self.text = ''
        self.watcher = ''

        self.__home__()

    def __home__(self):
        self.textFieldEdit = QtGui.QTextEdit(self)
        self.textFieldEdit.resize(400,500)
        self.textFieldEdit.move(20,20)
        self.textFieldEdit.setStyleSheet("color: black; background-color: #FFFFFF")

        self.textFieldEditTwo = QtGui.QTextEdit(self)
        self.textFieldEditTwo.resize(200,300)
        self.textFieldEditTwo.move(430,30)
        self.textFieldEditTwo.setReadOnly(True)
        self.textFieldEditTwo.setStyleSheet("color: black; background-color: #FFFFFF")

        self.textFieldEditTwo.append('Active users:')

        f = open("temp/out.txt", "r")
        self.text = f.read()
        f.close()

        self.watcher = QtCore.QFileSystemWatcher(['temp/out.txt'])
        self.watcher.fileChanged.connect(self.__file_changed_edit_file__)

        self.watcherTwo = QtCore.QFileSystemWatcher(['temp/selecposother.txt'])
        self.watcherTwo.fileChanged.connect(self.__text__selection__changed__)

        self.watcherThree = QtCore.QFileSystemWatcher(['temp/activusr.txt'])
        self.watcherThree.fileChanged.connect(self.__active_users__)

        self.textFieldEdit.setPlainText(self.text)
        self.textFieldEdit.textChanged.connect(self.__text_field_edit_event_func__)
        self.textFieldEdit.selectionChanged.connect(self.__text_selection__by__user__)
        self.show()

    def __active_users__(self):
        f = open("temp/activusr.txt", "r")
        text = f.read()
        f.close()
        try:
            userNum = int(text)
        except:
            userNum = 0

        if (userNum != 0) and (userNum < 10):
            self.textFieldEditTwo.clear()
            self.textFieldEditTwo.setTextColor(QtGui.QColor("#000000"))
            self.textFieldEditTwo.append('Active users:')
            for i in range(userNum):
                self.textFieldEditTwo.setTextColor(QtGui.QColor(self.colorsUsers[i]))
                self.textFieldEditTwo.append('User_%d' % (i))
        else:
            self.textFieldEditTwo.clear()
            self.textFieldEditTwo.setTextColor(QtGui.QColor("#000000"))
            self.textFieldEditTwo.append('Active users:')
            self.textFieldEditTwo.append('0')

    def __text__selection__changed__(self):
        self.textFieldEdit.textChanged.disconnect()
        format = QtGui.QTextCharFormat()
        cursor_old = self.textFieldEdit.textCursor() # User == self
        cursor_new = self.textFieldEdit.textCursor() # User == remote, who select text

        f = open("temp/selecposother.txt", "r")
        text = f.read()
        f.close()

        text = text.split()
        try:
            userNum = int(text[0])
        except:
            userNum = 0
        if(userNum != 0):
            temp_count = 1
            cursor_new.setPosition(0)
            cursor_new.setPosition(len(self.textFieldEdit.toPlainText()), QtGui.QTextCursor.KeepAnchor)
            format.setBackground(QtGui.QBrush(QtGui.QColor("#FFFFFF")))
            cursor_new.mergeCharFormat(format)
            self.textFieldEdit.setTextCursor(cursor_new)
            for i in range(userNum):
                cst = int(text[temp_count])
                temp_count += 1
                ced = int(text[temp_count])
                temp_count += 1
                if cst != ced:
                    cursor_new.setPosition(cst)
                    cursor_new.setPosition(ced, QtGui.QTextCursor.KeepAnchor)
                    format.setBackground(QtGui.QBrush(QtGui.QColor(self.colorsUsers[i])))
                    cursor_new.mergeCharFormat(format)
                    self.textFieldEdit.setTextCursor(cursor_new)
                else:
                    cursor_new.setPosition(cst)
                    cursor_new.setPosition(ced, QtGui.QTextCursor.KeepAnchor)
                    format.setBackground(QtGui.QBrush(QtGui.QColor("#FFFFFF")))
                    cursor_new.mergeCharFormat(format)
                    self.textFieldEdit.setTextCursor(cursor_new)

        format.setBackground(QtGui.QBrush(QtGui.QColor("#FFFFFF")))
        cursor_old.mergeCharFormat(format)
        self.textFieldEdit.setTextCursor(cursor_old)
        self.textFieldEdit.textChanged.connect(self.__text_field_edit_event_func__)

    def __text_selection__by__user__(self):
        f = open("temp/selecpos.txt", "w")
        f.write("%d\n%d" % (self.textFieldEdit.textCursor().selectionStart(), self.textFieldEdit.textCursor().selectionEnd()))
        f.close()

    def __text_field_edit_event_func__(self):
        #self.textFieldEdit.setFontWeight(QtGui.QFont.Normal)
        self.watcher.fileChanged.disconnect()
        self.textFieldEdit.textChanged.disconnect()

        self.text = self.textFieldEdit.toPlainText()
        copyCursor = self.textFieldEdit.textCursor()
        cursorPosition = self.textFieldEdit.textCursor().position()
        if self.text:
            self.fixSizeString = True
            self.text = self.text.lower()           # Change chars to lower
            textCopy = self.text.encode().decode()
            i = 0
            j = 0
            self.text = ''
            try:
                while(j < len(textCopy)):
                    if i==44:
                        if textCopy[j] != '\n':
                            self.text = self.text + '\n'
                        else:
                            self.text = self.text + textCopy[j]
                            j += 1
                        i = 0
                    if textCopy[j] == '\n':
                        i = 0
                    if(j < len(textCopy)):
                        self.text = self.text + textCopy[j]
                    i += 1
                    j += 1
            except:
                pass

            if self.fixSizeString == True:
                countNewLine = 0
                countChars = 0
                for i in range(len(self.text)):
                    countChars += 1
                    if self.text[i]=='\n':
                        countNewLine += 1
                    if countNewLine == 26:
                        break
                if countNewLine == 26:
                    self.text = self.text[:countChars]
                elif len(self.text) >= 1170: #45 chars in full line, 26 lines
                    self.text = self.text[:1170]
                fixSizeString = False

        self.textFieldEdit.setPlainText(self.text)
        try:
            copyCursor.setPosition(cursorPosition)
            self.textFieldEdit.setTextCursor(copyCursor)
        except:
            copyCursor.setPosition(0)
            self.textFieldEdit.setTextCursor(copyCursor)
        self.textFieldEdit.textChanged.connect(self.__text_field_edit_event_func__)
        with open("temp/out.txt", "w+") as f:
            f.write(self.text)
        self.watcher.fileChanged.connect(self.__file_changed_edit_file__)

    def __file_changed_edit_file__(self):
        self.textFieldEdit.textChanged.disconnect()
        f = open("temp/out.txt", "r")
        self.text = f.read()
        f.close()
        self.text = self.text.split('\n')
        copyCursor = self.textFieldEdit.textCursor()
        cursorPosition = self.textFieldEdit.textCursor().position()
        self.textFieldEdit.clear()
        for i in range(len(self.text)):
            self.textFieldEdit.append(self.text[i])
            i = i * 2

        try:
            copyCursor.setPosition(cursorPosition)
            self.textFieldEdit.setTextCursor(copyCursor)
        except:
            copyCursor.setPosition(0)
            self.textFieldEdit.setTextCursor(copyCursor)
        self.textFieldEdit.textChanged.connect(self.__text_field_edit_event_func__)

    def closeEvent(self, event):
        self.textFieldEdit.textChanged.disconnect()
        self.watcher.fileChanged.disconnect()
        self.watcherTwo.fileChanged.disconnect()
        self.watcherThree.fileChanged.disconnect()
        os.remove("temp/out.txt")
        os.remove("temp/selecpos.txt")
        os.remove("temp/activusr.txt")
        os.remove("temp/selecposother.txt")
        os.rmdir("temp")
        super(Window, self).closeEvent(event)


if __name__ == '__main__':
    if not os.path.exists("temp/out.txt"):
        os.makedirs("temp")
        f = open("temp/out.txt", "w")
        f.close()
        f = open("temp/selecpos.txt", "w")
        f.write("0\n0")
        f.close()
        f = open("temp/selecposother.txt", "w")
        f.write("0\n0\n0")  # How many users select text, start & end select
        f.close()
        f = open("temp/activusr.txt", "w")
        f.write("0")
        f.close()
    subprocess.Popen(["./client"])
    app = QtGui.QApplication(sys.argv)
    GUI = Window()
    sys.exit(app.exec_())

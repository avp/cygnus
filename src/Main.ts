import {
  BrowserWindow,
  Menu,
  dialog,
} from 'electron';
import { readFile } from 'fs';

import Puzzle from './Puzzle';

export default class Main {
  static mainWindow: Electron.BrowserWindow;
  static application: Electron.App;
  static BrowserWindow;

  static puzzle: Puzzle;

  private static onWindowAllClosed() {
    if (process.platform !== 'darwin') {
      Main.application.quit();
    }
  }

  private static onClose() {
    // Dereference the window object.
    Main.mainWindow = null;
  }

  private static openFile() {
    dialog.showOpenDialog(Main.mainWindow, {
    }, (files: string[]) => {
      if (files.length == 0) {
        return;
      }
      readFile(files[0], (err, data) => {
        if (err) {
          return console.error(err);
        }
        Main.puzzle = Puzzle.loadFromFile(data);
        Main.mainWindow.webContents.send('loadFile', Main.puzzle);
      });
    });
  }

  private static makeAppMenu() {
    const template = [
      {
        label: 'File',
        submenu: [
          {
            label: 'Open',
            accelerator: 'CommandOrControl+O',
            click: Main.openFile
          }
        ],
      },
    ];
    let appMenu = Menu.buildFromTemplate(template);
    Menu.setApplicationMenu(appMenu);
  }

  private static onReady() {
    Main.mainWindow = new Main.BrowserWindow({ width: 800, height: 600 })
    Main.mainWindow.maximize();
    Main.mainWindow.loadURL('file://' + __dirname + '/index.html');
    Main.mainWindow.isFullScreen();
    Main.mainWindow.on('closed', Main.onClose);
    Main.mainWindow.webContents.openDevTools();
    Main.makeAppMenu();
  }

  static main(
    app: Electron.App,
    browserWindow: typeof BrowserWindow) {
    // we pass the Electron.App object and the
    // Electron.BrowserWindow into this function
    // so this class1 has no dependencies.  This
    // makes the code easier to write tests for

    Main.BrowserWindow = browserWindow;
    Main.application = app;
    Main.application.on('window-all-closed', Main.onWindowAllClosed);
    Main.application.on('ready', Main.onReady);
  }
}

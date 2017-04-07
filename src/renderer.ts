import { ipcRenderer } from 'electron';
import * as _ from 'lodash';
import Main from './Main';
import Puzzle from './Puzzle';

enum Direction {
  ACROSS,
    DOWN,
}

export default class Renderer {
  private static cursor: {
    row: number,
    col: number,
    dir: Direction,
  } = null;

  private static puzzle: Puzzle = null;

  private static cells: HTMLElement[][];

  private static firstNonBlackCell() {
    let puzzle = Renderer.puzzle;
    for (let r = 0; r < puzzle.height; ++r) {
      for (let c = 0; c < puzzle.width; ++c) {
        if (puzzle.grid[r][c] !== null) {
          return { row: r, col: c };
        }
      }
    }
  }

  public static renderPuzzle(puzzle: Puzzle) {
    Renderer.puzzle = puzzle;

    let cell = Renderer.firstNonBlackCell();
    Renderer.cursor = {
      row: cell.row,
      col: cell.col,
      dir: Direction.ACROSS,
    };

    let puzzleDiv = document.getElementById('puzzle');
    puzzleDiv.innerHTML = '';

    let table = document.createElement('table');
    let tableBody = document.createElement('tbody');
    Renderer.cells = [];

    for (let r = 0; r < puzzle.height; ++r) {
      let row = document.createElement('tr');
      let cellRow = [];
      for (let c = 0; c < puzzle.width; ++c) {
        let cell = document.createElement('td');
        cell.className = 'puzzle-cell';
        if (puzzle.grid[r][c] === null) {
          cell.className += ' puzzle-cell-black';
        } else {
          cell.className += ' puzzle-cell-white';
          cell.innerHTML = puzzle.grid[r][c];
          if (puzzle.nums[r][c] !== null) {
            let num = document.createElement('span');
            num.className = 'puzzle-cell-num';
            num.innerHTML = String(puzzle.nums[r][c]);
            cell.appendChild(num);
          }
        }
        cell.style.height =
          String(puzzleDiv.clientHeight / puzzle.height) + 'px';
        cell.style.width =
          String(puzzleDiv.clientWidth / puzzle.width) + 'px';
        row.appendChild(cell);
        cellRow.push(cell);
      }
      tableBody.appendChild(row);
      Renderer.cells.push(cellRow);
    }

    table.appendChild(tableBody);
    puzzleDiv.appendChild(table);

    Renderer.renderClues();
    Renderer.refresh();
  }

  private static renderClues() {
    let puzzle = Renderer.puzzle;

    let acrossDiv = document.getElementById('clues-across');
    acrossDiv.innerHTML = '';

    let downDiv = document.getElementById('clues-down');
    downDiv.innerHTML = '';

    let acrossList = document.createElement('ol');
    for (let i = 0; i < puzzle.clues.across.length; ++i) {
      let li = document.createElement('li');
      li.setAttribute('value', String(puzzle.clues.across[i].num));
      li.innerHTML = puzzle.clues.across[i].clue;
      li.classList.add('clue');
      acrossList.appendChild(li);
    }
    acrossDiv.appendChild(acrossList);

    let downList = document.createElement('ol');
    for (let i = 0; i < puzzle.clues.down.length; ++i) {
      let li = document.createElement('li');
      li.setAttribute('value', String(puzzle.clues.down[i].num));
      li.innerHTML = puzzle.clues.down[i].clue;
      li.classList.add('clue');
      downList.appendChild(li);
    }
    downDiv.appendChild(downList);
  }

  public static refresh() {
    _(document.getElementsByClassName('puzzle-cell-current')).eachRight(
      (e) => {
        e.classList.remove('puzzle-cell-current');
      }
    );
    _(document.getElementsByClassName('puzzle-cell-current-clue')).eachRight(
      (e) => {
        e.classList.remove('puzzle-cell-current-clue');
      }
    );

    let r = Renderer.cursor.row;
    let c = Renderer.cursor.col;
    Renderer.cells[r][c].classList.add('puzzle-cell-current');
    if (Renderer.cursor.dir === Direction.ACROSS) {
      for (let i = 1; i < Renderer.puzzle.width; ++i) {
        if (c - i < 0 || Renderer.puzzle.grid[r][c-i] === null) {
          break;
        }
        if (Renderer.cells[r][c-i] !== null) {
          Renderer.cells[r][c-i].classList.add('puzzle-cell-current-clue');
        }
      }
      for (let i = 1; i < Renderer.puzzle.width; ++i) {
        if (c + i >= Renderer.puzzle.width ||
            Renderer.puzzle.grid[r][c+i] === null) {
          break;
        }
        if (Renderer.cells[r][c+i] !== null) {
          Renderer.cells[r][c+i].classList.add('puzzle-cell-current-clue');
        }
      }
    } else {
      for (let i = 1; i < Renderer.puzzle.height; ++i) {
        if (r - i < 0 || Renderer.puzzle.grid[r-i][c] === null) {
          break;
        }
        if (Renderer.cells[r-i][c] !== null) {
          Renderer.cells[r-i][c].classList.add('puzzle-cell-current-clue');
        }
      }
      for (let i = 1; i < Renderer.puzzle.height; ++i) {
        if (r + i >= Renderer.puzzle.height ||
            Renderer.puzzle.grid[r+i][c] === null) {
          break;
        }
        if (Renderer.cells[r+i][c] !== null) {
          Renderer.cells[r+i][c].classList.add('puzzle-cell-current-clue');
        }
      }
    }
  }

  public static upKey() {
    let cursor = Renderer.cursor;
    const puzzle = Renderer.puzzle;
    if (cursor.dir === Direction.ACROSS) {
      cursor.dir = Direction.DOWN;
    } else {
      do {
        if (cursor.row > 0) {
          --cursor.row;
        } else {
          break;
        }
      } while (puzzle.grid[cursor.row][cursor.col] === null);
    }
    Renderer.refresh();
  }

  public static downKey() {
    let cursor = Renderer.cursor;
    const puzzle = Renderer.puzzle;
    if (cursor.dir === Direction.ACROSS) {
      cursor.dir = Direction.DOWN;
    } else {
      do {
        if (cursor.row < puzzle.height - 1) {
          ++cursor.row;
        } else {
          break;
        }
      } while (puzzle.grid[cursor.row][cursor.col] === null);
    }
    Renderer.refresh();
  }

  public static rightKey() {
    let cursor = Renderer.cursor;
    const puzzle = Renderer.puzzle;
    if (cursor.dir === Direction.DOWN) {
      cursor.dir = Direction.ACROSS;
    } else {
      do {
        if (cursor.col < puzzle.width - 1) {
          ++cursor.col;
        } else {
          break;
        }
      } while (puzzle.grid[cursor.row][cursor.col] === null);
    }
    Renderer.refresh();
  }

  public static leftKey() {
    let cursor = Renderer.cursor;
    const puzzle = Renderer.puzzle;
    if (cursor.dir === Direction.DOWN) {
      cursor.dir = Direction.ACROSS;
    } else {
      do {
        if (cursor.col > 0) {
          --cursor.col;
        } else {
          break;
        }
      } while (puzzle.grid[cursor.row][cursor.col] === null);
    }
    Renderer.refresh();
  }


}

ipcRenderer.on('loadFile', (e, puzzle: Puzzle) => {
  Renderer.renderPuzzle(puzzle);
});

window.onkeydown = (e) => {
  console.log(e);
  switch (e.keyCode) {
    case 37: // left
      Renderer.leftKey();
      break;
    case 38: // up
      Renderer.upKey();
      break;
    case 39: // right
      Renderer.rightKey();
      break;
    case 40: // down
      Renderer.downKey();
      break;
  }
};

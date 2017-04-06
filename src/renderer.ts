import { ipcRenderer } from 'electron';
import Main from './Main';
import Puzzle from './Puzzle';

export default class Renderer {
  public static renderPuzzle(puzzle: Puzzle) {
    let puzzleDiv = document.getElementById('puzzle');
    puzzleDiv.innerHTML = '';

    let table = document.createElement('table');
    let tableBody = document.createElement('tbody');

    for (let r = 0; r < puzzle.height; ++r) {
      let row = document.createElement('tr');
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
            console.log(r, c, puzzle.nums[r][c]);
            num.innerHTML = String(puzzle.nums[r][c]);
            cell.appendChild(num);
          }
        }
        cell.style.height = String(puzzleDiv.clientHeight / puzzle.height) + 'px';
        cell.style.width = String(puzzleDiv.clientWidth / puzzle.width) + 'px';
        row.appendChild(cell);
      }
      tableBody.appendChild(row);
    }

    table.appendChild(tableBody);
    puzzleDiv.appendChild(table);
  }
}

ipcRenderer.on('loadFile', (e, puzzle: Puzzle) => {
  Renderer.renderPuzzle(puzzle);
});

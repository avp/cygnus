import { ipcRenderer } from 'electron';
import Main from './Main';
import Puzzle from './Puzzle';

ipcRenderer.on('loadFile', (e, puzzle: Puzzle) => {
  let puzzleDiv = document.getElementById('puzzle');
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
        cell.innerHTML = puzzle.grid[r][c];
      }
      cell.style.height = String(puzzleDiv.clientHeight / puzzle.height) + 'px';
      cell.style.width = String(puzzleDiv.clientWidth / puzzle.width) + 'px';
      row.appendChild(cell);
    }
    tableBody.appendChild(row);
  }

  table.appendChild(tableBody);
  puzzleDiv.appendChild(table);
});

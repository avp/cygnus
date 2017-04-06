const MAGIC: Buffer = Buffer.from(
    [0x41, 0x43, 0x52, 0x4f, 0x53, 0x53, 0x26, 0x44, 0x4f, 0x57, 0x4e, 0x00]);

interface Clue {
  clue: string;
  row: number;
  col: number;
  num: number;
}

export default class Puzzle {
  public width: number;
  public height: number;

  public solution: string[][];
  public grid: string[][];
  public nums: number[][];

  public across: Clue[];
  public down: Clue[];
  public clues: { across: Clue[], down: Clue[] };

  private static readGrid(buf: Buffer, width: number,
                          height: number): string[][] {
    let grid: string[][] = [];
    for (let r = 0; r < height; ++r) {
      let row: string[] = [];
      for (let c = 0; c < width; ++c) {
        let entry: string = String.fromCharCode(buf.readUInt8(r * width + c));
        if (entry === '.') {
          row.push(null);
        } else if (entry === '-') {
          row.push('');
        } else {
          row.push(entry);
        }
      }
      grid.push(row);
    }
    return grid;
  }

  private static readString(buf: Buffer, offset: number): string {
    let chars: number[] = [];
    for (let i = offset; buf[i] != 0; ++i) {
      chars.push(buf.readUInt8(i));
    }
    return String.fromCharCode.apply(String, chars);
  }

  public static loadFromFile(buf: Buffer): Puzzle {
    let fileChecksum = buf.readUInt16LE(0);
    let magic = buf.slice(0x02, 0x0e);
    if (!magic.equals(MAGIC)) {
      console.error('magic check failed:', magic);
      return null;
    }

    let width = buf.readUInt8(0x2c);
    let height = buf.readUInt8(0x2d);

    let numClues = buf.readUInt16LE(0x2e);
    let solution =
        Puzzle.readGrid(buf.slice(0x34, 0x34 + width * height), width, height);
    let grid = Puzzle.readGrid(
        buf.slice(0x34 + width * height, 0x34 + 2 * width * height), width,
        height);

    let offset = 0x34 + 2 * width * height;
    let title = Puzzle.readString(buf, offset);
    offset += title.length + 1;
    let author = Puzzle.readString(buf, offset);
    offset += author.length + 1;
    let copyright = Puzzle.readString(buf, offset);
    offset += copyright.length + 1;

    let num = 1;
    let across: Clue[] = [];
    let down: Clue[] = [];
    let nums: number[][] = [];
    for (let r = 0; r < height; ++r) {
      let numRow: number[] = [];
      for (let c = 0; c < width; ++c) {
        if (grid[r][c] === null) {
          numRow.push(null);
          continue;
        }
        let a: boolean = c === 0 || grid[r][c - 1] === null;
        let d: boolean = r === 0 || grid[r - 1][c] === null;
        if (a || d) {
          numRow.push(num);
          ++num;
          if (a) {
            let clue: Clue = {
              row: r,
              col: c,
              num: num,
              clue: Puzzle.readString(buf, offset)
            };
            offset += clue.clue.length + 1;
            across.push(clue);
          }
          if (d) {
            let clue: Clue = {
              row: r,
              col: c,
              num: num,
              clue: Puzzle.readString(buf, offset)
            };
            offset += clue.clue.length + 1;
            down.push(clue);
          }
        } else {
          numRow.push(null);
        }
      }
      nums.push(numRow);
    }

    return new Puzzle({
      width: width,
      height: height,
      solution: solution,
      grid: grid,
      nums: nums,
      clues: {
        across: across,
        down: down,
      },
    });
  }

  private constructor(opts) {
    this.width = opts.width;
    this.height = opts.height;
    this.solution = opts.solution;
    this.grid = opts.grid;
    this.nums = opts.nums;
    this.clues = opts.clues;
  }
}

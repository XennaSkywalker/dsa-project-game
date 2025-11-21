// Get canvas and context
const canvas = document.getElementById("gameCanvas");
const ctx = canvas.getContext("2d");

// Game grid settings
const WIDTH = 60;
const HEIGHT = 20;
const CELL_WIDTH = 20;
const CELL_HEIGHT = 20;

// Set canvas size to match grid
canvas.width = WIDTH * CELL_WIDTH;
canvas.height = HEIGHT * CELL_HEIGHT;

// Define level layout (same as your C++ backend)
const level = Array.from({ length: HEIGHT }, () => Array(WIDTH).fill(" "));

// Ground
for (let x = 0; x < WIDTH; x++) level[HEIGHT - 1][x] = "#";

// Platforms
for (let x = 10; x < 20; x++) level[14][x] = "#";
for (let x = 23; x < 35; x++) level[11][x] = "#";
for (let x = 37; x < 50; x++) level[8][x] = "#";
for (let x = 29; x < 33; x++) level[7][x] = "#";
for (let x = 15; x < 25; x++) level[5][x] = "#";

// Small step on left
for (let x = 5; x < 10; x++) level[17][x] = "#";

// Walls
for (let y = 0; y < HEIGHT; y++) {
  level[y][0] = "#";
  level[y][WIDTH - 1] = "#";
}

// Goal
const goal = { x: 15, y: 4 };

// Player object
let player = { x: 4, y: HEIGHT - 2 };

// Draw everything
function draw() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  // Draw platforms/walls
  for (let y = 0; y < HEIGHT; y++) {
    for (let x = 0; x < WIDTH; x++) {
      if (level[y][x] === "#") {
        ctx.fillStyle = "black";
        ctx.fillRect(x * CELL_WIDTH, y * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT);
      }
    }
  }

  // Draw goal
  ctx.fillStyle = "blue";
  ctx.fillRect(
    goal.x * CELL_WIDTH,
    goal.y * CELL_HEIGHT,
    CELL_WIDTH,
    CELL_HEIGHT
  );

  // Draw player
  ctx.fillStyle = "red";
  ctx.fillRect(
    player.x * CELL_WIDTH,
    player.y * CELL_HEIGHT,
    CELL_WIDTH,
    CELL_HEIGHT
  );
}

// Fetch game state from backend
async function update() {
  try {
    const res = await fetch("/state");
    const data = await res.json();
    player = { x: data.x, y: data.y };

    // Check for goal
    if (player.x === goal.x && player.y === goal.y) {
      alert("LEVEL COMPLETE!");
    }

    draw();
  } catch (e) {
    console.error("Could not fetch game state:", e);
  }
}

// Send input to backend
function sendInput(dir) {
  fetch(`/input?dir=${dir}`);
}

// Handle keyboard input
document.addEventListener("keydown", (e) => {
  if (e.key === "ArrowLeft") sendInput("left");
  if (e.key === "ArrowRight") sendInput("right");
  if (e.key === " ") sendInput("jump");
});

// Update loop (sync with backend 50ms)
setInterval(update, 50);

// Initial draw
draw();

const canvas = document.getElementById("gameCanvas");
const ctx = canvas.getContext("2d");
const messageDiv = document.getElementById("message");

const TILE_SIZE = 20;
let width = 60; // updated width
let height = 20; // updated height

let gameState = {};

// Flash message helper
function flashMessage(text, duration = 2000) {
  messageDiv.textContent = text;
  setTimeout(() => {
    messageDiv.textContent = "";
  }, duration);
}

// Send input to backend
function sendInput(key) {
  fetch("/input", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ key }),
  });
}

// Draw the grid-based game
function drawGame(data) {
  ctx.clearRect(0, 0, canvas.width, canvas.height);

  for (let y = 0; y < data.height; y++) {
    for (let x = 0; x < data.width; x++) {
      let tile = data.grid[y][x];
      if (tile === "#") ctx.fillStyle = "#888"; // wall/platform
      else if (tile === "P") ctx.fillStyle = "blue"; // player
      else if (tile === "G") ctx.fillStyle = "red"; // goal
      else ctx.fillStyle = "#333"; // empty

      ctx.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
    }
  }
}

// Fetch game state and render
const FPS = 10; // target frames per second
const FRAME_INTERVAL = 1000 / FPS;

let lastTime = 0;

async function update(time = 0) {
  const delta = time - lastTime;

  if (delta > FRAME_INTERVAL) {
    lastTime = time;

    try {
      const res = await fetch("/state");
      const data = await res.json();
      width = data.width;
      height = data.height;
      gameState = data;
      drawGame(data);
    } catch (err) {
      console.error(err);
    }
  }

  requestAnimationFrame(update);
}

// Keyboard controls with flash messages
document.addEventListener("keydown", (e) => {
  if (e.key === "ArrowLeft") sendInput("left");
  else if (e.key === "ArrowRight") sendInput("right");
  else if (e.key === "ArrowUp") sendInput("up");
  else if (e.key === "s") {
    sendInput("save");
    flashMessage("Game saved!");
  } else if (e.key === "u") {
    sendInput("undo");
    flashMessage("Undo last move!");
  } else if (e.key === "r") {
    sendInput("replay");
    flashMessage("Replay has started!");
  }
});

// Start game loop
update();

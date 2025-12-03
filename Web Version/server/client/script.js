const canvas = document.getElementById("gameCanvas");
const ctx = canvas.getContext("2d");
const messageDiv = document.getElementById("message");

const TILE_SIZE = 30;
let width = 50;
let height = 20;

let gameState = {};

function getRandomInteger(min, max) {
  min = Math.ceil(min);
  max = Math.floor(max);
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

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
  const playerImg = new Image();
  // const wallImg = new Image();
  const platformImg = new Image();
  const goalImg = new Image();
  const backgroundImg = new Image();
  const topImg = new Image();

  playerImg.src = "assets/player.png";
  // wallImg.src = "assets/wall.png";
  platformImg.src = "assets/platform.png";
  goalImg.src = "assets/goal.png";
  backgroundImg.src = "assets/background.png";
  topImg.src = "assets/top.png";

  ctx.clearRect(0, 0, canvas.width, canvas.height);
  for (let y = 0; y < data.height; y++) {
    for (let x = 0; x < data.width; x++) {
      let tile = data.grid[y][x];

      //wall
      // if (tile === "!") {
      //   ctx.fillStyle = "red";
      //   ctx.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
      //   ctx.drawImage(
      //     wallImg,
      //     x * TILE_SIZE,
      //     y * TILE_SIZE,
      //     TILE_SIZE,
      //     TILE_SIZE
      //   );
      // }

      //platform
      if (tile === "#") {
        ctx.drawImage(
          platformImg,
          x * TILE_SIZE,
          y * TILE_SIZE,
          TILE_SIZE,
          TILE_SIZE
        );
      }

      // player
      else if (tile === "P") {
        ctx.fillStyle = "#2EB082";
        ctx.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
        ctx.drawImage(
          playerImg,
          x * TILE_SIZE,
          y * TILE_SIZE,
          TILE_SIZE,
          TILE_SIZE
        );
      }

      // goal
      else if (tile === "G") {
        ctx.fillStyle = "#7FF0A5";
        ctx.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
        ctx.drawImage(
          goalImg,
          x * TILE_SIZE,
          y * TILE_SIZE,
          TILE_SIZE,
          TILE_SIZE
        );
      }

      //background
      else if (y > 10) {
        ctx.fillStyle = "#2EB082";
        ctx.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
      }

      //bg top
      else if (y === 10) {
        const randomInt = getRandomInteger(1, 5);
        ctx.fillStyle = "#7FF0A5";
        ctx.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
        ctx.drawImage(
          topImg,
          x * TILE_SIZE,
          y * TILE_SIZE,
          TILE_SIZE,
          TILE_SIZE
        );
      }

      //remaining
      else {
        ctx.fillStyle = "#7FF0A5";
        ctx.fillRect(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
      }
    }
  }
}

// Fetch game state and render
const FPS = 15; // target frames per second
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
  else if (e.key === "s" || e.key === "S") {
    sendInput("save");
    flashMessage("Game saved!");
  } else if (e.key === "u" || e.key === "U") {
    sendInput("undo");
    flashMessage("Undo last move!");
  } else if (e.key === "e" || e.key === "E") {
    sendInput("replay");
    flashMessage("Replay has started!");
  }
});

// Start game loop
update();

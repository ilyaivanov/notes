import { easeInCubic, expSmoothTowards, setMag } from "../vec";
import "./style.css";

const canvas = document.createElement("canvas");
const ctx = canvas.getContext("2d")!;
let width = 0;
let height = 0;

let zoom = 1;
let targetZoom = 1;
const cameraPos = { x: 0, y: 0 };
const cameraSpeed = 20;
const zoomSpeed = 1000;
const keys = new Set<string>();

type Card = {
  x: number;
  y: number;
  fromLevel: number;
  toLevel: number;
  timeToTransition: number;
};

function card(x: number, y: number): Card {
  return { x, y, fromLevel: 1, toLevel: 1, timeToTransition: 0 };
}

const cards: Card[] = [card(200, 100), card(-200, -50)];

function updateCanvasDimensions() {
  const scale = window.devicePixelRatio || 1;

  canvas.style.width = `${width}px`;
  canvas.style.height = `${height}px`;
  canvas.width = Math.floor(width * scale);
  canvas.height = Math.floor(height * scale);
  ctx.scale(scale, scale);
}

const transitionTimeSec = 0.3;
function switchLevelTo(card: Card, level: number) {
  card.fromLevel = card.toLevel;
  card.toLevel = level;
  card.timeToTransition = transitionTimeSec;
}

function updateCard(card: Card, deltaSec: number) {
  if (zoom >= 5) {
    if (card.toLevel != 3) switchLevelTo(card, 3);
  } else if (zoom >= 2) {
    if (card.toLevel != 2) switchLevelTo(card, 2);
  } else if (zoom < 2) {
    if (card.toLevel != 1) switchLevelTo(card, 1);
  }

  if (card.timeToTransition > 0)
    card.timeToTransition = Math.max(card.timeToTransition - deltaSec, 0);
}

function drawContent(x: number, y: number, size: number, level: number) {
  ctx.textBaseline = "middle";
  ctx.textAlign = "center";
  ctx.fillStyle = "white";

  if (level == 3) {
    ctx.font = "8px monospace";
    const cells = 3;
    for (let i = -1; i < cells - 1; i++)
      for (let j = -1; j < cells - 1; j++)
        ctx.fillText(
          "Hi Vadym",
          x - (size / (cells + 1)) * i,
          y - (size / (cells + 1)) * j,
        );
  } else if (level == 2) {
    ctx.font = "14px monospace";
    ctx.fillText("Level 2", x - size / 4, y - size / 4);
    ctx.fillText("Level 2", x + size / 4, y + size / 4);
    ctx.fillText("Level 2", x - size / 4, y + size / 4);
    ctx.fillText("Level 2", x + size / 4, y - size / 4);
  } else if (level == 1) {
    ctx.font = "20px monospace";
    ctx.fillText("Level 1", x, y);
  }
}

const cardSize = 150;
function drawCard(card: Card) {
  const { x, y } = card;

  ctx.fillStyle = "#222233";
  ctx.fillRect(x - cardSize / 2, y - cardSize / 2, cardSize, cardSize);
  if (card.timeToTransition > 0) {
    const animC = card.timeToTransition / transitionTimeSec;
    ctx.globalAlpha = easeInCubic(card.timeToTransition / transitionTimeSec);
    // ctx.globalAlpha = animC;
    drawContent(x, y, cardSize, card.fromLevel);

    ctx.globalAlpha = easeInCubic(1 - animC);
    // ctx.globalAlpha = 1 - animC;
    drawContent(x, y, cardSize, card.toLevel);
  } else {
    drawContent(x, y, cardSize, card.toLevel);
  }
  ctx.globalAlpha = 1;
}

function drawRectCentered(x: number, y: number, size: number, color: string) {
  const topLeftX = x - size / 2;
  const topLeftY = y - size / 2;

  ctx.fillStyle = color;
  ctx.fillRect(topLeftX, topLeftY, size, size);
}

function onWheel(delta: number) {
  targetZoom *= Math.exp(delta * -0.001);
}

function update(dt: number) {
  zoom = expSmoothTowards(zoom, targetZoom, 15, dt);
}

function drawLine() {
  ctx.lineCap = "round";
  ctx.lineWidth = 2;
  ctx.beginPath();
  const x1 = cards[1].x + cardSize / 2;
  const y1 = cards[1].y;
  const x2 = cards[0].x - cardSize / 2;
  const y2 = cards[0].y;
  ctx.moveTo(x1 + 1, y1);
  ctx.bezierCurveTo(
    x1 + Math.abs(x1 - x2) / 2,
    y1,
    x2 - Math.abs(x1 - x2) / 2,
    y2,
    x2 - 1,
    y2,
  );
  ctx.stroke();
}
let lastTime = 0;
function renderApp(time: number) {
  const deltaMs = time - lastTime;
  const deltaSec = deltaMs / 1000.0;

  let movement = { x: 0, y: 0 };

  if (keys.has("KeyW")) onWheel(-zoomSpeed * deltaSec);
  if (keys.has("KeyS")) onWheel(zoomSpeed * deltaSec);

  update(deltaSec);

  if (keys.has("KeyK")) movement.y = -1;
  if (keys.has("KeyI")) movement.y = 1;
  if (keys.has("KeyJ")) movement.x = 1;
  if (keys.has("KeyL")) movement.x = -1;

  movement = setMag(movement, cameraSpeed / zoom);
  cameraPos.x += movement.x;
  cameraPos.y += movement.y;

  ctx.clearRect(0, 0, width, height);

  ctx.save();

  // const sc = 0.5 + (Math.sin(time / 1000) + 1) / 2;

  ctx.translate(width / 2, height / 2);
  ctx.scale(zoom, zoom);
  ctx.translate(cameraPos.x, cameraPos.y);

  ctx.lineWidth = 2;
  ctx.strokeStyle = "white";

  drawLine();

  for (let i = 0; i < cards.length; i++) {
    updateCard(cards[i], deltaSec);
    drawCard(cards[i]);
  }

  ctx.restore();

  ctx.save();

  ctx.fillStyle = "white";
  ctx.textAlign = "right";
  ctx.textBaseline = "bottom";
  ctx.font = "14px monospace";
  ctx.fillText("Zoom: " + zoom.toFixed(4), width - 20, height - 20);
  ctx.fillText(
    "FPS: " + (1000 / (time - lastTime)).toFixed(0),
    width - 20,
    height - 20 - 20,
  );
  ctx.restore();

  lastTime = time;
  requestAnimationFrame(renderApp);
}

function onResize() {
  width = window.innerWidth;
  height = window.innerHeight;

  updateCanvasDimensions();
  // renderApp();
}

document.body.appendChild(canvas);
onResize();

window.addEventListener("resize", onResize);
document.addEventListener("keydown", (e) => keys.add(e.code));
document.addEventListener("keyup", (e) => keys.delete(e.code));

requestAnimationFrame(renderApp);

export type v2 = { x: number; y: number };
export function setMag(v: v2, magnitude: number): v2 {
  const len = Math.hypot(v.x, v.y);

  // Avoid division by zero
  if (len === 0) {
    return { x: 0, y: 0 };
  }

  const scale = magnitude / len;
  return {
    x: v.x * scale,
    y: v.y * scale,
  };
}

export function expSmoothTowards(
  current: number,
  target: number,
  lambda: number,
  dt: number,
): number {
  // lambda = convergence speed (e.g. 8–20 feels good)
  const t = 1 - Math.exp(-lambda * dt);
  return current + (target - current) * t;
}

//easings
export function easeOutCubic(x: number): number {
  return 1 - Math.pow(1 - x, 3);
}

export function easeInCubic(x: number): number {
  return x * x * x;
}

export function easeOutBounce(x: number): number {
  const n1 = 7.5625;
  const d1 = 2.75;

  if (x < 1 / d1) {
    return n1 * x * x;
  } else if (x < 2 / d1) {
    return n1 * (x -= 1.5 / d1) * x + 0.75;
  } else if (x < 2.5 / d1) {
    return n1 * (x -= 2.25 / d1) * x + 0.9375;
  } else {
    return n1 * (x -= 2.625 / d1) * x + 0.984375;
  }
}

export function easeInExpo(x: number): number {
  return x === 0 ? 0 : Math.pow(2, 10 * x - 10);
}

export function easeOutExpo(x: number): number {
  return x === 1 ? 1 : 1 - Math.pow(2, -10 * x);
}

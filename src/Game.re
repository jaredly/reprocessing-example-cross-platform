open Reprocessing;

type state = {
  titleFont: fontT,
  statusFont: fontT,
  player: imageT,
  star: imageT,
  left: bool,
  hasInteracted: bool,
  points: int,
  starPos: (float, float),
  pos: (float, float),
  velocity: (float, float),
  t: float
};

let playerWidth = 200 / 2;
let playerHeight = 133 / 2;
let starSize = 100 / 2;

let groundColor = Utils.color(~r=183, ~g=114, ~b=59, ~a=255);
let groundHeight = 100;

let randomStarPos = env => {
  (
    Random.float(float_of_int(Env.width(env) - starSize)),
    Random.float(float_of_int(Env.height(env) - groundHeight - starSize)),
  )
};

let setup = (assetDir, env) => {
  if (Reprocessing.target != "native-android" && Reprocessing.target != "native-ios") {
    Env.size(~width=340, ~height=620, env);
  };
  {
    t: 0.,
    points: 0,
    left: false,
    hasInteracted: false,
    velocity: (0., 0.),
    starPos: randomStarPos(env),
    pos: (0., float_of_int(Env.height(env) - playerHeight - groundHeight)),
    star: Draw.loadImage(~filename=Filename.concat(assetDir, "star.png"), env),
    player: Draw.loadImage(~filename=Filename.concat(assetDir, "player2.png"), env),
    statusFont: Draw.loadFont(~filename=Filename.concat(assetDir, "SFCompactDisplay-Regular-16.fnt"), env),
    titleFont: Draw.loadFont(~filename=Filename.concat(assetDir, "Arial Black-32.fnt"), env),
  }
};

let isJumping = env => {
  if (Env.mousePressed(env)) {
    let (x, y) = Env.mouse(env);
    if (y < Env.height(env) / 2) {
      true
    } else {
      (Env.key(Events.Up, env))
    }
  } else {
    (Env.key(Events.Up, env))
  }
};

let action = env => {
  if (Env.mousePressed(env)) {
    let (x, y) = Env.mouse(env);
    if (x < Env.width(env) / 3) {
      `Left
    } else if (x > Env.width(env) * 2 / 3) {
      `Right
    } else {
      `Nothing
    }
  } else if (Env.key(Events.Left, env)) {
    `Left
  } else if (Env.key(Events.Right, env)) {
    `Right
  } else {
    `Nothing
  }
};

let step = ({pos: (x, y) as pos, left, velocity: (vx, vy)} as state, env) => {
  let delta = Env.deltaTime(env) *. 60.;
  let maxY = float_of_int(Env.height(env) - groundHeight - playerHeight);
  let vy = y <= 0. ? max(0., vy) : vy;
  let y = min(max(y, 0.), maxY);
  let x = min(max(x, 0.), float_of_int(Env.width(env) - playerWidth));
  let pos = (x,y);
  let vx = vx *. (1.0 -. 0.1 *. delta);

  let ((vx, vy), (x, y)) = if (isJumping(env)) {
    /* if (y == maxY) { */
    /* Yay double jump */
    if (vy >= 0.) {
      ((vx, -15.), pos)
    } else {
      ((vx, vy), pos)
    }
  } else {
    if (y >= maxY) {
      ((vx, 0.), (x, maxY))
    } else {
      ((vx, vy), pos)
    }
  };

  let ((vx, vy), left) = switch (action(env)) {
  | `Nothing => ((vx, vy), left)
  | `Left => ((-4., vy), true)
  | `Right => ((4., vy), false)
  };
  let x = x +. vx *. delta;
  let y = y +. vy *. delta;

  let y = min(max(y, 0.), maxY);
  let vy = y >= maxY ? vy : vy +. 0.5 *. delta;
  let x = min(max(x, 0.), float_of_int(Env.width(env) - playerWidth));


  let gotStar = Utils.intersectRectRect(
    ~rect1Pos=(x, y),
    ~rect1H=float_of_int(playerHeight),
    ~rect1W=float_of_int(playerWidth),
    ~rect2Pos=state.starPos,
    ~rect2H=float_of_int(starSize),
    ~rect2W=float_of_int(starSize)
  );
  let (points, starPos) = gotStar
    ? (state.points + 1, randomStarPos(env))
    : (state.points, state.starPos);

  {
    ...state,
    left,
    points,
    starPos,
    hasInteracted: state.hasInteracted || (isJumping(env) || action(env) !== `Nothing),
    pos: (x, y),
    velocity: (vx, vy)
  }
};

let draw = ({left, pos, points, t} as state, env) => {
  let floorY = Env.height(env) - groundHeight;

  let state = step(state, env);

  Draw.background(Utils.color(~r=240, ~g=240, ~b=240, ~a=255), env);

  /* Floor */
  Draw.fill(groundColor, env);
  Draw.rect(~pos=(0, floorY), ~width=Env.width(env), ~height=groundHeight, env);

  let (x, y) = pos;
  let x = int_of_float(x);
  let y = int_of_float(y);
  Draw.image(state.player,
    ~pos=left ? (x + playerWidth, y) : (x, y),
    ~width=left ? -playerWidth : playerWidth,
    ~height=playerHeight,
    env
  );

  let (x, y) = state.starPos;
  Draw.image(state.star,
    ~pos=(int_of_float(x), int_of_float(y)),
    ~width=starSize,
    ~height=starSize,
    env
  );

  let (x, y) = Env.mouse(env);
  Draw.tint(Constants.black, env);
  Draw.text(
    ~font=state.statusFont,
    ~body="You have " ++ string_of_int(points) ++ " points",
    ~pos=(10, 5),
    env
  );

  Draw.tint(groundColor, env);
  if (!state.hasInteracted) {
    switch (state.titleFont^) {
    | None => ()
    | Some(font) =>
      let width = Reprocessing_Font.Font.calcStringWidth(
        env,
        font,
        "CamlQuest"
      );
      Draw.text(
        ~font=state.titleFont,
        ~body="CamlQuest",
        ~pos=(Env.width(env) / 2 - width / 2, 100),
        env
      );
    }
  };

  Draw.noTint(env);
  {...state, t: t +. Env.deltaTime(env)}
};

let run = (assetDir) => Reprocessing.run(~setup=setup(assetDir), ~draw, ~title="CamlQuest", ());
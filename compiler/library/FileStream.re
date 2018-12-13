type file_stream = {
  channel: in_channel,
  current: ref(char),
  row: ref(int),
  column: ref(int),
};

type file_cursor = {
  index: int,
  row: int,
  column: int,
};

let rec _next_valid_char = channel =>
  switch (input_char(channel)) {
  | '\r' => _next_valid_char(channel)
  | _ as ch => ch
  };

let load = path =>
  open_in(path)
  |> (
    channel => {
      channel,
      current: ref(char_of_int(0)),
      row: ref(1),
      column: ref(0),
    }
  );

let close = ({channel}) => close_in(channel);

let count = ({channel}) => pos_in(channel);

let length = ({channel}) => in_channel_length(channel);

let rec next = ({channel} as stream) =>
  try (Some(advance(stream))) {
  | End_of_file
  | _ => None
  }
and advance = ({channel, row, column} as stream) => {
  let char = input_char(channel);

  switch (char) {
  | '\n' =>
    let curs = {index: pos_in(channel), row: row^, column: column^ + 1};
    stream.column := 0;
    stream.row := row^ + 1;
    (char, curs);
  | _ =>
    stream.column := column^ + 1;

    switch (char) {
    | '\r' => advance(stream)
    | _ => (char, {index: pos_in(channel), row: row^, column: column^})
    };
  };
};

let junk = stream => next(stream) |> ignore;

let rec njunk = (n, stream) =>
  if (n == 0) {
    ();
  } else {
    junk(stream);
    njunk(n - 1, stream);
  };

let peek = ({channel}) =>
  try (
    Some(
      {
        let index = pos_in(channel);
        let char = _next_valid_char(channel);

        seek_in(channel, index);
        char;
      },
    )
  ) {
  | _ => None
  };

let npeek = (n, {channel}) =>
  try (
    {
      let index = pos_in(channel);

      let rec loop = (chs, i) =>
        if (i == 0) {
          Some(chs);
        } else {
          try (loop([_next_valid_char(channel), ...chs], i - 1)) {
          | _ => loop(chs, 0)
          };
        };
      let string = loop([], n);

      seek_in(channel, index);
      string;
    }
  ) {
  | _ => None
  };

let reposition = ({channel} as stream, {index, row, column}) => {
  seek_in(channel, index - 1);

  switch (input_char(channel)) {
  | '\r' =>
    stream.row := row;
    stream.column := column;
    advance(stream) |> ignore;
  | '\n' =>
    stream.row := row + 1;
    stream.column := 0;
  | _ =>
    stream.row := row;
    stream.column := column;
  };
};
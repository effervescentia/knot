type file_stream = {
  channel: in_channel,
  row: ref(int),
  column: ref(int),
};

type file_cursor = {
  index: int,
  row: int,
  column: int,
};

let load = path =>
  open_in(path) |> (channel => {channel, row: ref(1), column: ref(0)});

let close = ({channel}) => close_in(channel);

let rec next = ({channel} as stream) =>
  try (Some(advance(stream))) {
  | End_of_file =>
    close_in(channel);
    None;
  | _ => None
  }
and advance = ({channel, row, column} as stream) => {
  let char = input_char(channel);

  switch (char) {
  | '\n' as ch =>
    stream.column := 0;
    stream.row := row^ + 1;
    ch;
  | _ as ch =>
    stream.column := column^ + 1;

    switch (ch) {
    | '\r' => advance(stream)
    | _ => ch
    };
  };
};

let junk = stream => {
  let _ = next(stream);
  ();
};

let cursor = ({channel, row, column}) => {
  index: pos_in(channel),
  row: row^,
  column: column^,
};

let reposition = ({channel} as stream, {index, row, column}) => {
  seek_in(channel, index);
  stream.row := row;
  stream.column := column;
};
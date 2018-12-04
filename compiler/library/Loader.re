type char_stream = {
  channel: in_channel,
  row: ref(int),
  column: ref(int),
};

let load = path =>
  open_in(path) |> (channel => {channel, row: ref(0), column: ref(0)});

let rec next = ({channel} as stream) =>
  try (Some(advance(stream))) {
  | End_of_file => None
  }
and advance = ({channel, row, column} as stream) =>
  switch (input_char(channel)) {
  | '\r' => advance(stream)
  | '\n' as ch =>
    stream.row := row^ + 1;
    stream.column := 0;
    ch;
  | _ as ch =>
    stream.column := column^ + 1;
    ch;
  };

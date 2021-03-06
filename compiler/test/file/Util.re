open Kore;

let write_to_file = (path, s) => {
  let out = open_out(path);
  Print.fprintf(out, "%s", s);

  close_out(out);
};

let read_channel_to_string = in_ => {
  let string = ref(input_line(in_));

  let rec loop = () =>
    switch (string := string^ ++ input_line(in_) ++ "\n") {
    | _ => loop()
    | exception End_of_file =>
      close_in(in_);
      string^;
    };

  loop();
};

let read_file_to_string = path => {
  let in_ = open_in(path);
  let string = read_channel_to_string(in_);

  close_in(in_);

  string;
};

let read_lazy_char_stream = stream => {
  let buffer = Buffer.create(1);

  let rec loop = s => {
    switch (s) {
    | LazyStream.Cons((c, _), next) =>
      Buffer.add_utf_8_uchar(buffer, c);
      Lazy.force(next) |> loop;
    | LazyStream.Nil => Buffer.contents(buffer)
    };
  };

  loop(stream);
};

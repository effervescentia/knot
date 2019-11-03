open Globals;

let _uchar_buffer_size = 128;

let rec range = (start, end_) =>
  if (start >= end_) {
    [];
  } else {
    [start, ...range(start + 1, end_)];
  };

let print_sequential = (~separator="", printer, xs) => {
  let rec next =
    fun
    | [x] => printer(x)
    | [x, ...xs] => printer(x) ++ separator ++ next(xs)
    | [] => "";

  next(xs);
};

let print_comma_separated = printer =>
  print_sequential(~separator=", ", printer);

let print_optional = printer =>
  fun
  | Some(x) => printer(x)
  | None => "";

let print_uchar = ch => {
  let buf = Buffer.create(_uchar_buffer_size);
  Buffer.add_utf_8_uchar(buf, ch);

  Buffer.contents(buf);
};

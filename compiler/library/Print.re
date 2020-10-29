open Operators;

let _uchar_buffer_size = 128;

let sequential = (~separator="", printer, xs) => {
  let rec next =
    fun
    | [x] => printer(x)
    | [x, ...xs] => printer(x) ++ separator ++ next(xs)
    | [] => "";

  next(xs);
};

let comma_separated = printer => sequential(~separator=", ", printer);

let optional = (~default="", printer) =>
  fun
  | Some(x) => printer(x)
  | None => default;

let uchar = ch => {
  let buf = Buffer.create(_uchar_buffer_size);
  Buffer.add_utf_8_uchar(buf, ch);

  Buffer.contents(buf);
};

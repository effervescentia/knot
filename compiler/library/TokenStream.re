type token_stream = {
  file_stream: FileStream.file_stream,
  curr: ref(option((char, FileStream.file_cursor))),
  ctx: ref(list(Lexer.context)),
};

let load = file_stream => {file_stream, curr: ref(None), ctx: ref([])};

let count = ({file_stream}) => FileStream.count(file_stream);

let next = ({file_stream, ctx} as stream) =>
  switch (FileStream.next(file_stream)) {
  | Some(res) =>
    stream.curr := Some(res);
    Some(Lexer.lex(res, ctx, file_stream));
  | None => None
  };

let reposition =
    ({file_stream}, {index, row, column}: FileStream.file_cursor) =>
  FileStream.reposition(file_stream, {index, row, column});

let peek = ({file_stream, curr} as stream) => {
  let file_cursor =
    switch (curr^) {
    | Some((_, cursor)) => cursor
    | None => {index: 0, row: 1, column: 0}
    };

  let token = next(stream);

  reposition(stream, file_cursor);
  token;
};

let push_ctx = ({ctx}, next_ctx) => ctx := [next_ctx, ...ctx^];
let pop_ctx = ({ctx}) =>
  List.length(ctx^) == 0 ? ctx := List.tl(ctx^) : ();
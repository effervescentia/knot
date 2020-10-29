open Lwt.Infix;

module Config = Httpaf.Config;

module Buffer = {
  type t = {
    buffer: Lwt_bytes.t,
    mutable off: int,
    mutable len: int,
  };

  let create = size => {
    let buffer = Lwt_bytes.create(size);

    {buffer, off: 0, len: 0};
  };

  let compress = t =>
    if (t.len == 0) {
      t.off = 0;
      t.len = 0;
    } else if (t.off > 0) {
      Lwt_bytes.blit(t.buffer, t.off, t.buffer, 0, t.len);
      t.off = 0;
    };

  let get = (t, ~f) => {
    let n = f(t.buffer, ~off=t.off, ~len=t.len);

    t.off = t.off + n;
    t.len = t.len - n;

    if (t.len == 0) {
      t.off = 0;
    };

    n;
  };

  let put = (t, ~f) => {
    compress(t);

    f(t.buffer, ~off=t.off + t.len, ~len=Lwt_bytes.length(t.buffer) - t.len)
    >>= (
      n => {
        t.len = t.len + n;
        Lwt.return(n);
      }
    );
  };
};

let read = (fd, buffer) =>
  Lwt.catch(
    () =>
      Buffer.put(buffer, ~f=(bigstring, ~off, ~len) =>
        Lwt_bytes.read(fd, bigstring, off, len)
      ),
    fun
    | Unix.Unix_error(Unix.EBADF, _, _) as exn => Lwt.fail(exn)
    | exn => {
        Lwt.async(() => Lwt_unix.close(fd));
        Lwt.fail(exn);
      },
  )
  >>= (
    bytes_read =>
      bytes_read == 0 ? Lwt.return(`Eof) : Lwt.return(`Ok(bytes_read))
  );

let shutdown = (socket, command) =>
  try (Lwt_unix.shutdown(socket, command)) {
  | Unix.Unix_error(Unix.ENOTCONN, _, _) => ()
  };

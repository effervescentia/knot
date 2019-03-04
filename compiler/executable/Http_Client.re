open Lwt.Infix;
open Http_Util;

module HTTP = Httpaf.Client_connection;

let request =
    (
      ~config=Config.default,
      socket,
      request,
      ~error_handler,
      ~response_handler,
    ) => {
  let (request_body, connection) =
    HTTP.request(request, ~error_handler, ~response_handler);

  let read_buffer = Buffer.create(config.response_body_buffer_size);
  let (read_loop_exited, notify_read_loop_exited) = Lwt.wait();

  let read_loop = () => {
    let rec read_loop_step = () =>
      switch (HTTP.next_read_operation(connection)) {
      | `Read =>
        read(socket, read_buffer)
        >>= (
          fun
          | `Eof => {
              Buffer.get(read_buffer, ~f=(bigstring, ~off, ~len) =>
                HTTP.read_eof(connection, bigstring, ~off, ~len)
              )
              |> ignore;

              read_loop_step();
            }
          | `Ok(_) => {
              Buffer.get(read_buffer, ~f=(bigstring, ~off, ~len) =>
                HTTP.read(connection, bigstring, ~off, ~len)
              )
              |> ignore;

              read_loop_step();
            }
        )
      | `Close =>
        Lwt.wakeup_later(notify_read_loop_exited, ());

        if (Lwt_unix.state(socket) != Lwt_unix.Closed) {
          shutdown(socket, Unix.SHUTDOWN_RECEIVE);
        };

        Lwt.return_unit;
      };

    Lwt.async(() =>
      Lwt.catch(
        read_loop_step,
        exn => {
          HTTP.report_exn(connection, exn);
          Lwt.return_unit;
        },
      )
    );
  };

  let writev = Faraday_lwt_unix.writev_of_fd(socket);
  let (write_loop_exited, notify_write_loop_exited) = Lwt.wait();

  let rec write_loop = () => {
    let rec write_loop_step = () =>
      switch (HTTP.next_write_operation(connection)) {
      | `Write(io_vectors) =>
        writev(io_vectors)
        >>= (
          result => {
            HTTP.report_write_result(connection, result);
            write_loop_step();
          }
        )
      | `Yield =>
        HTTP.yield_writer(connection, write_loop);

        Lwt.return_unit;
      | `Close(_) =>
        Lwt.wakeup_later(notify_write_loop_exited, ());

        Lwt.return_unit;
      };

    Lwt.async(() =>
      Lwt.catch(
        write_loop_step,
        exn => {
          HTTP.report_exn(connection, exn);
          Lwt.return_unit;
        },
      )
    );
  };

  read_loop();
  write_loop();

  Lwt.async(() =>
    Lwt.join([read_loop_exited, write_loop_exited])
    >>= (
      () =>
        if (Lwt_unix.state(socket) != Lwt_unix.Closed) {
          Lwt.catch(() => Lwt_unix.close(socket), _exn => Lwt.return_unit);
        } else {
          Lwt.return_unit;
        }
    )
  );

  request_body;
};

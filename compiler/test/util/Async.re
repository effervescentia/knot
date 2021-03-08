module Tester = Alcotest_engine.Cli.Make(Alcotest.Unix, Lwt);

include Tester;

let test_case_sync = (n, s, f) => test_case(n, s, x => Lwt.return(f(x)));

let run_test = (fn, args) => {
  let (async_ex, async_waker) = Lwt.wait();
  let handle_exn = ex => {
    Printf.sprintf("Uncaught async exception: %s", Printexc.to_string(ex))
    |> print_endline;

    if (Lwt.state(async_ex) == Lwt.Sleep) {
      Lwt.wakeup_exn(async_waker, ex);
    };
  };
  Lwt.async_exception_hook := handle_exn;
  Lwt_switch.with_switch(sw => Lwt.pick([fn(sw, args), async_ex]));
};

let test_case = (n, s, f) => test_case(n, s, run_test(f));

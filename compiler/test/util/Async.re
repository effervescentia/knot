module Tester = Alcotest_engine.Cli.Make(Alcotest.Unix, Lwt);

include Tester;

exception AsyncFailure;

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

let on_tick = (~ticks=1, callback) => {
  let timeout = Lwt_timeout.create(ticks, callback);

  Lwt_timeout.start(timeout);

  () => Lwt_timeout.stop(timeout);
};

let wait = () => {
  let (promise, resolver) = Lwt.wait();

  (promise, Lwt.wakeup(resolver));
};

let delay = (timeout, f) =>
  Lwt.(async(() => Lwt_unix.sleep(timeout) >>= (() => return(f()))));

let fail_after = timeout =>
  Lwt.(Lwt_unix.sleep(timeout) >>= (() => fail(AsyncFailure)));

let await_success = (~ticks=2, ()) => {
  let (promise, resolve) = wait();
  let cancel =
    on_tick(~ticks, () => Assert.fail("test did not finish in time"));
  let resolve' = () => {
    cancel();
    resolve();
  };

  (promise, resolve');
};

let await_failure = (~ticks=2, ()) => {
  let (promise, resolve) = wait();
  let cancel = on_tick(~ticks, resolve);
  let reject = err => {
    cancel();
    Assert.fail(err);
  };

  (promise, reject);
};

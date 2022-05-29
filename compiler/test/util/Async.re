include Alcotest_lwt;

exception AsyncFailure;

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

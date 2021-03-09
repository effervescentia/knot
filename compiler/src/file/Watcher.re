open Kore;
open Fswatch;
open Lwt;

/**
 watches a directory structure and reacts to files changing
 */
type t = {
  dir: string,
  extensions: list(string),
};

type action_t =
  | Add
  | Remove
  | Update
  | Relocate;

type dispatch_t = list((string, action_t)) => unit;

let _ext_matches = (path: string, watcher: t): bool =>
  watcher.extensions |> List.exists(ext => String.ends_with(ext, path));

let rec _listen = (dispatch: dispatch_t, watcher: t, msgBox) =>
  Lwt_mvar.take(msgBox)
  >>= (
    events => {
      events
      |> Array.to_list
      |> List.filter_map(
           Event.(
             event => {
               let path = event.path |> String.drop_prefix(watcher.dir);

               (
                 switch (event.flags) {
                 | flags when !Array.mem(IsFile, flags) => None
                 | flags when !(watcher |> _ext_matches(event.path)) => None
                 | flags when Array.mem(Created, flags) => Some(Add)
                 | flags when Array.mem(Updated, flags) => Some(Update)
                 | flags when Array.mem(Removed, flags) => Some(Remove)
                 | flags
                     when
                       [Renamed, MovedFrom, MovedTo]
                       |> List.exists(flag => Array.mem(flag, flags)) =>
                   Some(Relocate)
                 | _ => Some(Update)
                 }
               )
               |> (
                 fun
                 | Some(action) => Some((path, action))
                 | None => None
               );
             }
           ),
         )
      |> dispatch;

      flush(stdout);
      _listen(dispatch, watcher, msgBox);
    }
  );

/* static */

let create = (dir: string, extensions: list(string)) => {
  switch (init_library()) {
  | Status.FSW_OK => ()
  | status => raise(WatchFailed(Status.t_to_string(status)))
  };

  {dir, extensions};
};

/* methods */

/**
 start watching a folder for file system events
 */
let watch = (dispatch: dispatch_t, watcher: t) => {
  let (handle, msgBox) = Fswatch_lwt.init_session(Monitor.System_default);

  add_path(handle, watcher.dir);

  Lwt.async(Fswatch_lwt.start_monitor(handle));

  _listen(dispatch, watcher, msgBox);
};

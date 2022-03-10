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

type dispatch_t = list((string, FilesystemDriver.action_t)) => unit;

let _ext_matches = (path: string, watcher: t): bool =>
  watcher.extensions |> List.exists(ext => String.ends_with(ext, path));

let _flag_to_string: Fswatch.Event.flag => string =
  Fswatch.Event.(
    fun
    | NoOp => "NoOp"
    | PlatformSpecific => "PlatformSpecific"
    | Created => "Created"
    | Updated => "Updated"
    | Removed => "Removed"
    | Renamed => "Renamed"
    | OwnerModified => "OwnerModified"
    | AttributeModified => "AttributeModified"
    | MovedFrom => "MovedFrom"
    | MovedTo => "MovedTo"
    | IsFile => "IsFile"
    | IsDir => "IsDir"
    | IsSymLink => "IsSymLink"
    | Link => "Link"
    | Overflow => "Overflow"
  );

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
               let path = path == "" ? "/" : path;

               print_endline("file found: " ++ path);
               print_endline(
                 "flags: "
                 ++ (
                   event.flags
                   |> Array.map(_flag_to_string)
                   |> Array.fold_left((++), "")
                 ),
               );

               FilesystemDriver.handle(event.path, event.flags)
               |> Option.map(action => (path, action));
             }
           ),
         )
      |> dispatch;

      /*
        TODO: might have to change this back to stdout
        the thinking is that all log messages are written to stderr so shouldn't have
        anything to flush on stdout
       */
      flush(stderr);
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

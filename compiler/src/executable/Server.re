open Kore;

type server_config = {
  name: string,
  root_dir: string,
  port: int,
  watch: option(watch_config),
}
and watch_config = {entry: m_id};

let run = (cfg: server_config) => {
  ();
};

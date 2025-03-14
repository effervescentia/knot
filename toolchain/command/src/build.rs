use crate::{log, Logger};
use engine::engine2::{Builder, Context, Engine, Input, Peek, State, Transform};
use engine::{ConfigurationError, Report};
use kore::invariant;
use kore::{color::Highlight, internal, pretty::Pretty};
use notify_debouncer_full::{
    new_debouncer,
    notify::{self, Watcher},
    DebouncedEvent,
};
use std::path::Path;
use std::sync::mpsc;
use std::time::Duration;

const WATCH_SENSITIVITY: Duration = Duration::from_millis(20);

pub struct Options<'a, Platform> {
    pub platform: Platform,
    pub out_dir: &'a Path,
    pub source_dir: &'a Path,
    pub entry: &'a Path,

    /// enables a higher level of logging for additional information
    pub verbose: bool,
}

fn build_plan<'a, Platform>(
    opts: &Options<'_, Platform>,
) -> Builder<impl Transform<In = (State<'a, Logger>, ()), Out = (State<'a, Logger>, usize)>>
where
    Platform: internal::Platform<Program = lang::ast::shape::Program>,
{
    Engine::<Logger>::plan()
        .parse()
        .peek(|(_, x)| Logger.report_parsed(x))
        .link()
        .peek(|(_, x)| Logger.report_linked(x))
        .analyze()
        .peek(|(_, x)| Logger.report_analyzed(x))
        .generate(Platform::generator())
        .write(opts.out_dir)
}

pub fn command<Platform>(opts: &Options<Platform>) -> engine::Result<()>
where
    Platform: internal::Platform<Program = lang::ast::shape::Program>,
{
    log::entrypoint(opts.verbose, opts.entry);

    let input = Input::from_entry(opts.entry, []);
    let engine = Engine::new(Context::new(opts.source_dir, Logger));
    let plan = build_plan(opts);

    let (_, count) = engine.execute(&plan, &input);

    log::success(opts.verbose, "transpiled", count);
    eprintln!(
        "build artifacts written to {}:\n{}\n",
        "out_dir".focus(),
        opts.out_dir.pretty()
    );

    Ok(())
}

pub fn watch_command<Platform>(opts: &Options<Platform>) -> engine::Result<()>
where
    Platform: internal::Platform<Program = lang::ast::shape::Program>,
{
    log::entrypoint(opts.verbose, opts.entry);

    let input = Input::from_entry(opts.entry, []);
    let engine = Engine::new(Context::new(opts.source_dir, Logger));
    let plan = build_plan(opts);

    let (mut state, count) = engine.execute(&plan, &input);

    log::success(opts.verbose, "transpiled", count);
    eprintln!(
        "build artifacts written to {}:\n{}\n",
        "out_dir".focus(),
        opts.out_dir.pretty()
    );

    let (tx, rx) = mpsc::channel::<Result<Vec<DebouncedEvent>, Vec<notify::Error>>>();

    let mut debouncer = new_debouncer(WATCH_SENSITIVITY, None, tx)
        .unwrap_or_else(|_| invariant!("failed to create debouncer"));

    debouncer
        .watcher()
        .watch(opts.source_dir, notify::RecursiveMode::Recursive)
        .map_err(|x| match x.kind {
            notify::ErrorKind::PathNotFound => Report::Configuration(
                ConfigurationError::SourceDirectoryNotFound(opts.source_dir.to_path_buf()),
            ),

            notify::ErrorKind::MaxFilesWatch => Report::Environment(
                engine::EnvironmentError::MaxFilesWatched(opts.source_dir.to_path_buf()),
            ),

            notify::ErrorKind::Generic(reason) => Report::Environment(
                engine::EnvironmentError::WatchFailed(opts.source_dir.to_path_buf(), reason),
            ),

            err => Report::Environment(engine::EnvironmentError::WatchFailed(
                opts.source_dir.to_path_buf(),
                format!("{:?}", err),
            )),
        })?;

    debouncer
        .cache()
        .add_root(opts.source_dir, notify::RecursiveMode::Recursive);

    eprintln!("watching for changes in {}", opts.source_dir.pretty());
    eprintln!("press ctrl+c to cancel");

    for res in rx {
        match res {
            Ok(event) => {
                println!("event: {:?}", event);

                let operations = vec![];

                (state, _) = Engine::incremental(state, &plan, operations);
            }
            Err(e) => eprintln!("watch error: {:?}", e),
        }
    }

    Ok(())
}

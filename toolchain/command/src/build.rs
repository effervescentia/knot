use crate::{log, AssertExists, Logger};
use engine::{Builder, ConfigurationError, Context, Engine, Input, Report, State};
use kore::{
    color::Highlight,
    internal, invariant,
    pipeline::{Peek, Transform},
    pretty::Pretty,
};
use notify_debouncer_full::{
    new_debouncer,
    notify::{self, Watcher},
    DebouncedEvent,
};
use std::{path::Path, sync::mpsc, time::Duration};

const WATCH_SENSITIVITY: Duration = Duration::from_millis(20);

pub struct Options<'a, Platform> {
    pub platform: Platform,

    /// absolute path to the directory where build artifacts should be written
    pub out_dir: &'a Path,

    /// absolute path to the directory containing the source code
    pub source_dir: &'a Path,

    /// path to the entry file for this application or library
    /// relative to the `source_dir`
    pub entry: &'a Path,

    /// enables a higher level of logging for additional information
    pub verbose: bool,
}

fn build_plan<'a, Platform>(
    opts: &Options<'_, Platform>,
) -> Builder<impl Transform<Context = State<'a, Logger>, In = (), Out = usize>>
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
    let source_dir = opts
        .source_dir
        .assert_dir_exists(ConfigurationError::SourceDirectoryNotFound)?;

    log::entrypoint(opts.verbose, opts.entry);

    let input = Input::from_entry(opts.entry, Platform::libraries());
    let engine = Engine::new(Context::new(source_dir, Logger));
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
    let source_dir = opts
        .source_dir
        .assert_dir_exists(ConfigurationError::SourceDirectoryNotFound)?;

    log::entrypoint(opts.verbose, opts.entry);

    let input = Input::from_entry(opts.entry, Platform::libraries());
    let engine = Engine::new(Context::new(source_dir, Logger));
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
                ConfigurationError::SourceDirectoryNotFound(source_dir.to_path_buf()),
            ),

            notify::ErrorKind::MaxFilesWatch => Report::Environment(
                engine::EnvironmentError::MaxFilesWatched(source_dir.to_path_buf()),
            ),

            notify::ErrorKind::Generic(reason) => Report::Environment(
                engine::EnvironmentError::WatchFailed(source_dir.to_path_buf(), reason),
            ),

            err => Report::Environment(engine::EnvironmentError::WatchFailed(
                source_dir.to_path_buf(),
                format!("{:?}", err),
            )),
        })?;

    debouncer
        .cache()
        .add_root(source_dir, notify::RecursiveMode::Recursive);

    eprintln!("watching for changes in {}", source_dir.pretty());
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

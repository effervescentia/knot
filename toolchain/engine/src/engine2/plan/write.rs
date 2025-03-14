use crate::engine2::{logger::Logger, state::State};
use kore::pipeline::Transform;
use std::{
    fmt::Display,
    fs,
    io::{BufWriter, Write as _},
    path::PathBuf,
};

pub trait Output {
    type Data: Display;

    fn output(self) -> Vec<(PathBuf, Self::Data)>;
}

impl Output for Vec<(PathBuf, String)> {
    type Data = String;

    fn output(self) -> Vec<(PathBuf, Self::Data)> {
        self
    }
}

pub struct Write<Tx>(Tx, PathBuf);

impl<Tx> Write<Tx> {
    pub const fn new(tx: Tx, path: PathBuf) -> Self {
        Self(tx, path)
    }

    pub const fn bind(path: PathBuf) -> impl FnOnce(Tx) -> Self {
        |tx| Self(tx, path)
    }
}

impl<'a, Tx, Out, Log> Transform for Write<Tx>
where
    Tx: Transform<Context = State<'a, Log>, Out = Out>,
    Out: Output,
    Log: Logger + 'a,
{
    type Context = Tx::Context;
    type In = Tx::In;
    type Out = usize;

    fn apply(&self, input: (Self::Context, Self::In)) -> (Self::Context, Self::Out) {
        let (state, output) = self.0.apply(input);
        let mut count = 0;

        for (path, data) in output.output() {
            let absolute = self.1.join(path);

            if let Some(parent) = absolute.parent() {
                fs::create_dir_all(parent).unwrap();
            }

            let file = fs::File::create(&absolute).unwrap();

            let mut writer = BufWriter::new(file);

            write!(writer, "{data}").unwrap();
            writer.flush().unwrap();

            state
                .log()
                .debug(format_args!("\u{1f4be} emitted {}", absolute.display()));

            count += 1;
        }

        (state, count)
    }
}

#[cfg(test)]
mod tests {
    use super::Write;
    use crate::engine2::{logger::MemoryLogger, state::State, Context};
    use assert_fs::{
        assert::PathAssert,
        prelude::{FileWriteStr, PathChild},
        TempDir,
    };
    use kore::{
        assert_eq,
        pipeline::{Identity, Transform},
        str,
    };

    #[test]
    fn write_multiple_files() {
        let root_dir = TempDir::new().unwrap();
        let context = Context::new(&root_dir, MemoryLogger::default());

        let main_file = root_dir.child("main.kn");
        let foo_file = root_dir.child("foo.kn");
        let bar_file = root_dir.child("bar.kn");

        let (state, count) = Write::new(Identity::new(), root_dir.to_path_buf()).apply((
            State::mock(&context),
            vec![
                (main_file.to_path_buf(), str!("MAIN")),
                (foo_file.to_path_buf(), str!("FOO")),
                (bar_file.to_path_buf(), str!("BAR")),
            ],
        ));

        assert_eq!(count, 3);
        main_file.assert("MAIN");
        foo_file.assert("FOO");
        bar_file.assert("BAR");
        state.log().assert(&[
            (
                str!("debug"),
                format!("\u{1f4be} emitted {}", main_file.display()),
            ),
            (
                str!("debug"),
                format!("\u{1f4be} emitted {}", foo_file.display()),
            ),
            (
                str!("debug"),
                format!("\u{1f4be} emitted {}", bar_file.display()),
            ),
        ]);
    }

    #[test]
    fn overwrite_file() {
        let root_dir = TempDir::new().unwrap();
        let context = Context::new(&root_dir, MemoryLogger::default());

        let file = root_dir.child("main.kn");

        file.write_str("OLD").unwrap();

        let (_, count) = Write::new(Identity::new(), root_dir.to_path_buf()).apply((
            State::mock(&context),
            vec![(file.to_path_buf(), str!("NEW"))],
        ));

        assert_eq!(count, 1);
        file.assert("NEW");
    }

    #[test]
    fn create_directories() {
        let root_dir = TempDir::new().unwrap();
        let context = Context::new(&root_dir, MemoryLogger::default());

        let file = root_dir.child("foo/bar/main.kn");

        Write::new(Identity::new(), root_dir.to_path_buf()).apply((
            State::mock(&context),
            vec![(file.to_path_buf(), str!("CONTENTS"))],
        ));

        file.assert("CONTENTS");
    }
}

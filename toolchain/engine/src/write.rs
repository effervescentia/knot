use crate::{EnvironmentError, Report, Result};
use std::{
    fmt::Display,
    fs::{self, File},
    io::{BufWriter, Write},
    path::{Path, PathBuf},
};

pub struct Writer<T>
where
    T: Display,
{
    pub files: Result<Vec<(PathBuf, T)>>,
    pub verbose: bool,
}

impl<T> Writer<T>
where
    T: Display,
{
    pub fn overwrite(self, dir: &Path) -> Result<usize> {
        if dir.exists() {
            fs::remove_dir_all(dir).map_err(|err| {
                Report::Environment(EnvironmentError::CleanupFailed(
                    dir.to_path_buf(),
                    err.kind(),
                ))
            })?;
        }

        fs::create_dir_all(dir).ok();

        self.write(dir)
    }

    pub fn write(self, dir: &Path) -> Result<usize> {
        self.files.and_then(|files| {
            let mut count = 0;

            for (path, generated) in files {
                let path = dir.join(path);

                if let Some(parent) = path.parent() {
                    fs::create_dir_all(parent).ok();
                }

                let file = match File::create(&path) {
                    Ok(x) => Ok(x),

                    Err(err) => Err(Report::Environment(EnvironmentError::InvalidWriteTarget(
                        path.clone(),
                        err.kind(),
                    ))),
                }?;

                let mut writer = BufWriter::new(file);

                write!(writer, "{generated}").ok();
                writer.flush().ok();

                if self.verbose {
                    eprintln!("\u{1f4be} emitted {}", path.display());
                }

                count += 1;
            }

            Ok(count)
        })
    }
}

use crate::{ExecutionError, Reporter, Result};
use std::{
    fmt::Display,
    fs::{self, File},
    io::{BufWriter, Write},
    path::{Path, PathBuf},
};

pub struct Writer<T>(pub Result<Vec<(PathBuf, T)>>, pub Reporter)
where
    T: Display;

impl<T> Writer<T>
where
    T: Display,
{
    pub fn overwrite(mut self, dir: &Path) -> Result<usize> {
        if dir.exists() {
            fs::remove_dir_all(dir).map_err(|_| {
                self.1
                    .fail(ExecutionError::CleanupFailed(dir.to_path_buf()))
            })?
        }

        fs::create_dir_all(dir).ok();

        self.write(dir)
    }

    pub fn write(mut self, dir: &Path) -> Result<usize> {
        let mut count = 0;

        match &self.0 {
            Ok(xs) => {
                for (path, generated) in xs {
                    let path = dir.join(path);

                    if let Some(parent) = path.parent() {
                        fs::create_dir_all(parent).ok();
                    }

                    let file = match File::create(&path) {
                        Ok(x) => Ok(x),

                        Err(err) => {
                            self.1.raise(ExecutionError::InvalidWriteTarget(
                                path.clone(),
                                err.kind(),
                            ))?;
                            continue;
                        }
                    }?;

                    let mut writer = BufWriter::new(file);

                    write!(writer, "{generated}").ok();
                    writer.flush().ok();

                    count += 1;
                }

                self.1.flush()?;

                Ok(count)
            }

            Err(err) => Err(err.clone()),
        }
    }
}

use crate::{Error, Result};
use std::{
    fmt::Display,
    fs::{self, File},
    io::{BufWriter, Write},
    path::{Path, PathBuf},
};

pub struct Writer<T>(pub Result<Vec<(PathBuf, T)>>)
where
    T: Display;

impl<T> Writer<T>
where
    T: Display,
{
    pub fn write(&self, dir: &Path) -> Result<()> {
        match &self.0 {
            Ok(xs) => {
                for (path, generated) in xs {
                    let path = dir.join(path);

                    if let Some(parent) = path.parent() {
                        fs::create_dir_all(parent).ok();
                    }

                    let mut writer =
                        BufWriter::new(File::create(&path).map_err(|x| {
                            vec![Error::InvalidWriteTarget(path.clone(), x.kind())]
                        })?);

                    write!(writer, "{generated}").ok();
                    writer.flush().ok();
                }

                Ok(())
            }

            Err(err) => Err(err.clone()),
        }
    }
}

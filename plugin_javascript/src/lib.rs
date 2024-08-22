mod format;
mod javascript;
mod resolve;
#[cfg(test)]
mod test;
mod transform;

pub use javascript::JavaScript;
use kore::{str, Generator};
use lang::ast;
use resolve::ImportResolver;
use std::{
    fmt::Display,
    path::{Path, PathBuf},
    str::FromStr,
};

// TODO: move these to a common library to be re-used across generators

#[derive(PartialEq)]
pub enum Mode {
    Development,
    Production,
}

#[derive(Clone, Copy)]
pub enum Module {
    /// ECMAScript modules
    ESM,

    /// CommonJS modules
    CJS,
}

impl Default for Module {
    fn default() -> Self {
        Self::ESM
    }
}

pub struct Options<Library> {
    pub mode: Mode,
    pub resolver: ImportResolver<Library>,
}

#[derive(Clone, Copy)]
pub struct JavaScriptGenerator<Library>(ImportResolver<Library>);

impl<Library> JavaScriptGenerator<Library> {
    pub const fn new(module: Module) -> Self {
        Self(ImportResolver::new(module))
    }
}

impl<Library> Generator for JavaScriptGenerator<Library>
where
    Library: FromStr + Copy + Display,
{
    type Input = ast::shape::Program;
    type Output = JavaScript;

    fn generate(&self, path: &Path, input: Self::Input) -> (PathBuf, Self::Output) {
        let mut path_to_root = path
            .parent()
            .map(|x| x.iter().map(|_| "..").collect::<Vec<_>>().join("/"))
            .unwrap_or_default();
        if path_to_root.is_empty() {
            path_to_root = str!(".");
        }

        (
            path.with_extension("js"),
            JavaScript::from_program(
                &path_to_root,
                &input,
                &Options {
                    mode: Mode::Production,
                    resolver: self.0,
                },
            ),
        )
    }
}

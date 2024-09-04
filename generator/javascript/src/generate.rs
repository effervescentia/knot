use crate::{resolve::Resolver, JavaScript, Module, Options};
use kore::{internal, str};
use lang::ast as knot;
use std::path::{Path, PathBuf};

#[derive(Clone, Copy)]
pub struct Generator<Library>(Resolver<Library>);

impl<Library> Generator<Library> {
    pub const fn new(module: Module) -> Self {
        Self(Resolver::new(module))
    }
}

impl<Library> internal::Generator for Generator<Library>
where
    Library: internal::PlatformLibrary,
{
    type Input = knot::shape::Program;
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
                    mode: internal::Mode::Production,
                    resolver: self.0,
                },
            ),
        )
    }
}

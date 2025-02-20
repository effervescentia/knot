use super::{write::Output, Parsed};
use crate::engine2::{pipeline::Transform, state::State};
use kore::internal;
use lang::ast;
use std::path::PathBuf;

pub struct Generated(Vec<(PathBuf, String)>);

impl Output for Generated {
    type Data = String;

    fn output(self) -> Vec<(PathBuf, Self::Data)> {
        self.0
    }
}

pub struct Generate<Tx, Gen>(Tx, Gen);

impl<Tx, Gen> Generate<Tx, Gen> {
    pub const fn new(tx: Tx, generator: Gen) -> Self {
        Self(tx, generator)
    }

    pub const fn bind(generator: Gen) -> impl FnOnce(Tx) -> Self {
        |tx| Self(tx, generator)
    }
}

impl<'a, Tx, Res, Gen, Log> Transform for Generate<Tx, Gen>
where
    Tx: Transform<Out = (State<'a, Log>, Res)>,
    Res: Into<Parsed>,
    Gen: internal::Generator<Input = ast::shape::Program>,
    Log: 'a,
{
    type In = Tx::In;
    type Out = (State<'a, Log>, Generated);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, result) = self.0.apply(input);
        let Parsed(ids) = result.into();

        let output = ids
            .iter()
            .map(|id| {
                let module = state.get_module(id).unwrap();
                let (out_path, out_data) =
                    self.1.generate(&module.path, module.ast.clone().to_shape());

                (out_path, out_data.to_string())
            })
            .collect();

        (state, Generated(output))
    }
}

#[cfg(test)]
mod tests {
    use super::Generate;
    use crate::engine2::{
        logger::MemoryLogger,
        pipeline::{Identity, Transform},
        state::{State, Status},
        Analyzed, Context,
    };
    use assert_fs::TempDir;
    use kore::{assert_eq_sorted, internal, str};
    use lang::{ast, test::fixture, ModuleId};
    use std::{collections::HashSet, path::PathBuf};

    #[derive(Clone, Copy)]
    struct ToStringGenerator;

    impl internal::Generator for ToStringGenerator {
        type Input = ast::shape::Program;
        type Output = String;

        fn generate<T>(&self, path: T, input: Self::Input) -> (PathBuf, Self::Output)
        where
            T: AsRef<std::path::Path>,
        {
            (
                path.as_ref().with_extension("out"),
                format!("OUTPUT: {input}"),
            )
        }
    }

    fn mock_program(declaration: ast::raw::Declaration) -> ast::raw::Program {
        ast::meta::Program(ast::meta::Module::mock(ast::Module::new(
            vec![],
            vec![declaration],
        )))
    }

    fn mock_module<Log>(
        state: &mut State<Log>,
        path: &str,
        declaration: ast::raw::Declaration,
    ) -> ModuleId {
        state.create_module(path, str!(""), mock_program(declaration), Status::Active)
    }

    #[test]
    fn generate_all_analyzed_modules() {
        let root_dir = TempDir::new().unwrap();
        let context = Context::new(&root_dir, MemoryLogger::default());
        let mut state = State::mock(&context);

        let main_id = mock_module(&mut state, "main.kn", fixture::constant::mock());
        let foo_id = mock_module(&mut state, "foo.kn", fixture::function::mock());
        let bar_id = mock_module(&mut state, "bar.kn", fixture::view::mock());

        let (_, formatted) = Generate::new(Identity::new(), ToStringGenerator)
            .apply((state, Analyzed(HashSet::from([main_id, foo_id, bar_id]))));

        assert_eq_sorted!(
            HashSet::from_iter(formatted.0),
            HashSet::from([
                (
                    PathBuf::from("main.out"),
                    format!("OUTPUT: {}\n", fixture::constant::SOURCE)
                ),
                (
                    PathBuf::from("foo.out"),
                    format!("OUTPUT: {}\n", fixture::function::SOURCE)
                ),
                (
                    PathBuf::from("bar.out"),
                    format!("OUTPUT: {}\n", fixture::view::FORMATTED)
                )
            ])
        );
    }
}

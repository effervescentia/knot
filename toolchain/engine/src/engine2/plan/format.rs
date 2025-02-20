use super::{parse::Parsed, write::Output};
use crate::engine2::{pipeline::Transform, state::State};
use std::path::PathBuf;

pub struct Formatted(Vec<(PathBuf, String)>);

impl Output for Formatted {
    type Data = String;

    fn output(self) -> Vec<(PathBuf, Self::Data)> {
        self.0
    }
}

pub struct Format<Tx>(Tx);

impl<Tx> Format<Tx> {
    pub const fn new(tx: Tx) -> Self {
        Self(tx)
    }
}

impl<'a, Tx, Res, Log> Transform for Format<Tx>
where
    Tx: Transform<Out = (State<'a, Log>, Res)>,
    Res: Into<Parsed>,
    Log: 'a,
{
    type In = Tx::In;
    type Out = (State<'a, Log>, Formatted);

    fn apply(&self, input: Self::In) -> Self::Out {
        let (state, result) = self.0.apply(input);
        let Parsed(ids) = result.into();

        let output = ids
            .iter()
            .map(|id| {
                let module = state.get_module(id).unwrap();

                (module.path.clone(), module.ast.to_string())
            })
            .collect();

        (state, Formatted(output))
    }
}

#[cfg(test)]
mod tests {
    use super::Format;
    use crate::engine2::{
        logger::MemoryLogger,
        pipeline::{Identity, Transform},
        plan::parse::Parsed,
        state::{State, Status},
        Context,
    };
    use assert_fs::TempDir;
    use kore::{assert_eq_sorted, str};
    use lang::{ast, test::fixture, ModuleId};
    use std::{collections::HashSet, path::PathBuf};

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
    fn format_all_parsed_modules() {
        let root_dir = TempDir::new().unwrap();
        let context = Context::new(&root_dir, MemoryLogger::default());
        let mut state = State::mock(&context);

        let main_id = mock_module(&mut state, "main.kn", fixture::constant::mock());
        let foo_id = mock_module(&mut state, "foo.kn", fixture::function::mock());
        let bar_id = mock_module(&mut state, "bar.kn", fixture::view::mock());

        let (_, formatted) = Format::new(Identity::new())
            .apply((state, Parsed(HashSet::from([main_id, foo_id, bar_id]))));

        assert_eq_sorted!(
            HashSet::from_iter(formatted.0),
            HashSet::from([
                (
                    PathBuf::from("main.kn"),
                    format!("{}\n", fixture::constant::SOURCE)
                ),
                (
                    PathBuf::from("foo.kn"),
                    format!("{}\n", fixture::function::SOURCE)
                ),
                (
                    PathBuf::from("bar.kn"),
                    format!("{}\n", fixture::view::FORMATTED)
                )
            ])
        );
    }
}

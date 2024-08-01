use crate::{link::ImportGraph, state, Context, ExecutionError, InnerResult, Result};
use kore::invariant;

pub struct Validator<'a, R>(pub &'a mut Context<R>);

impl<'a, R> Validator<'a, R> {
    pub fn validate(self, state: &state::Parsed, graph: &ImportGraph) -> Result<()> {
        self.0.raise(self.assert_no_import_cycles(state, graph))
    }

    pub fn assert_no_import_cycles(
        &self,
        state: &state::Parsed,
        graph: &ImportGraph,
    ) -> InnerResult<()> {
        if !graph.is_cyclic() {
            return Ok(());
        }

        let errors = graph
            .cycles()
            .into_iter()
            .map(|x| {
                ExecutionError::ImportCycle(
                    x.to_vec()
                        .iter()
                        .map(|x| {
                            state
                                .get_link_by_id(x)
                                .unwrap_or_else(|| {
                                    invariant!("lookup did not contain module with id {x}")
                                })
                                .clone()
                        })
                        .collect(),
                )
            })
            .collect::<Vec<_>>();

        if errors.is_empty() {
            Ok(())
        } else {
            Err(errors)
        }
    }
}

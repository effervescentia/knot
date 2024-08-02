use crate::{link::ImportGraph, state, Context, ExecutionError};
use kore::invariant;

type Result = Option<Vec<ExecutionError>>;

pub struct Validator<'a, R>(pub &'a mut Context<R>);

impl<'a, R> Validator<'a, R> {
    pub fn validate(self, state: &state::Parsed, graph: &ImportGraph) -> crate::InternalResult<()> {
        let errors = vec![self.assert_no_import_cycles(state, graph)]
            .into_iter()
            .map(|x| x.unwrap_or_default())
            .flatten()
            .collect::<Vec<_>>();

        self.0.raise(errors)
    }

    fn assert_no_import_cycles(&self, state: &state::Parsed, graph: &ImportGraph) -> Result {
        if !graph.is_cyclic() {
            return None;
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
            None
        } else {
            Some(errors)
        }
    }
}

use crate::{graph::Graph, state, Context, ExecutionError};
use kore::invariant;

type Result = Option<Vec<ExecutionError>>;

pub struct Validator<'a, R>(pub &'a mut Context<R>);

impl<R> Validator<'_, R> {
    pub fn validate(self, state: &state::Parsed, graph: &Graph) -> crate::Internal<()> {
        let errors = vec![Self::assert_no_import_cycles(state, graph)]
            .into_iter()
            .flat_map(std::option::Option::unwrap_or_default)
            .collect::<Vec<_>>();

        self.0.raise(errors)
    }

    fn assert_no_import_cycles(state: &state::Parsed, graph: &Graph) -> Result {
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

use crate::{link::ImportGraph, state, ExecutionError, InnerResult};
use kore::invariant;

pub struct Validator<'a>(pub &'a state::Parsed);

impl<'a> Validator<'a> {
    pub fn assert_no_import_cycles(&self, graph: &ImportGraph) -> InnerResult<()> {
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
                            self.0
                                .get_link_by_id(x)
                                .unwrap_or_else(|| {
                                    invariant!("lookup did not contain module with id {x}")
                                })
                                .clone()
                        })
                        .collect::<Vec<_>>(),
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

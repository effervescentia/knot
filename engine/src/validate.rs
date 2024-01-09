use crate::{link::ImportGraph, state, Error, Result};
use kore::invariant;

pub struct Validator<'a>(pub &'a state::Parsed);

impl<'a> Validator<'a> {
    pub fn assert_no_import_cycles(&self, graph: &ImportGraph) -> Result<()> {
        if !graph.is_cyclic() {
            return Ok(());
        }

        let errors = graph
            .cycles()
            .into_iter()
            .map(|x| {
                Error::ImportCycle(
                    x.to_vec()
                        .iter()
                        .map(|x| {
                            self.0
                                .lookup
                                .get_by_right(x)
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

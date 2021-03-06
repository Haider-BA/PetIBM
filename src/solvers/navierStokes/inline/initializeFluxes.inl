/***************************************************************************//**
 * \file initializeFluxes.inl
 * \author Anush Kirshnan (anush@bu.edu), Olivier Mesnard (mesnardo@gwu.edu)
 * \brief Implementation of the method `initializeFluxes` 
 *        of the class `NavierStokesSolver`.
 */


/**
 * \brief Computes the initial fluxes.
 * 
 * Initialize the flux vector with the values of the velocity fluxes at the first
 * time step. The initial values of the velocity from which the fluxes are 
 * calculated are read from the FlowDescription object `flowDesc`.
 *
 * In the input file `simulationParameters.yaml`, if `startStep` is higher than 
 * `0`, the simulation is restarted, reading the numerical solution at the 
 * time-step specified.
 *
 * In the input file `flowDescription.yaml`, if the parameter `initialCustomField` 
 * is set to `true`, the initial field is read from the solution folder `0000000`.
 * However if the input `startStep` is higher than `0`, the simulation is considered
 * as restarted.
 */
template <PetscInt dim>
PetscErrorCode NavierStokesSolver<dim>::initializeFluxes()
{
  return 0;
} // initializeFluxes


// specialization in 2 dimensions
template <>
PetscErrorCode NavierStokesSolver<2>::initializeFluxes()
{
  PetscErrorCode ierr;
  
  if (parameters->startStep > 0 || flow->initialCustomField)
  {
    ierr = readFluxes(); CHKERRQ(ierr);
  }
  else
  { 
    PetscInt i, j,           // loop indices
             m, n,           // local number of nodes along each direction
             mstart, nstart; // starting indices

    // get access to individual packed vectors in their global representation
    Vec qxGlobal, qyGlobal;
    ierr = DMCompositeGetAccess(qPack, q, &qxGlobal, &qyGlobal); CHKERRQ(ierr);

    // fluxes in x-direction
    PetscReal **qx;
    ierr = DMDAVecGetArray(uda, qxGlobal, &qx); CHKERRQ(ierr);
    ierr = DMDAGetCorners(uda, &mstart, &nstart, NULL, &m, &n, NULL); CHKERRQ(ierr);
    for (j=nstart; j<nstart+n; j++)
    {
      for (i=mstart; i<mstart+m; i++)
      {
        qx[j][i] = flow->initialVelocity[0] * mesh->dy[j];
      }
    }
    ierr = DMDAVecRestoreArray(uda, qxGlobal, &qx); CHKERRQ(ierr);
    // fluxes in y-direction
    PetscReal **qy;
    ierr = DMDAVecGetArray(vda, qyGlobal, &qy); CHKERRQ(ierr);
    ierr = DMDAGetCorners(vda, &mstart, &nstart, NULL, &m, &n, NULL); CHKERRQ(ierr);
    for (j=nstart; j<nstart+n; j++)
    {
      for (i=mstart; i<mstart+m; i++)
      {
        qy[j][i] = flow->initialVelocity[1] * mesh->dx[i];
      }
    }
    ierr = DMDAVecRestoreArray(vda, qyGlobal, &qy); CHKERRQ(ierr);

    ierr = DMCompositeRestoreAccess(qPack, q, &qxGlobal, &qyGlobal); CHKERRQ(ierr);

    if (flow->perturbationAmplitude != 0.0)
    {
      ierr = addInitialPerturbation(); CHKERRQ(ierr);
    }
  }

  ierr = DMCompositeScatter(qPack, q, qxLocal, qyLocal); CHKERRQ(ierr);

  return 0;
} // initializeFluxes


// specialization in 3 dimensions
template <>
PetscErrorCode NavierStokesSolver<3>::initializeFluxes()
{
  PetscErrorCode ierr;

  if (parameters->startStep > 0 || flow->initialCustomField)
  {
    ierr = readFluxes(); CHKERRQ(ierr);
  }
  else
  {
    PetscInt i, j, k,                // loop indices
             m, n, p,                // local number of nodes along each direction
             mstart, nstart, pstart; // starting indices

    // get access to individual packed vectors in their global representation
    Vec qxGlobal, qyGlobal, qzGlobal;
    ierr = DMCompositeGetAccess(qPack, q, &qxGlobal, &qyGlobal, &qzGlobal); CHKERRQ(ierr);
    
    // fluxes in x-direction
    PetscReal ***qx;
    ierr = DMDAVecGetArray(uda, qxGlobal, &qx); CHKERRQ(ierr);
    ierr = DMDAGetCorners(uda, &mstart, &nstart, &pstart, &m, &n, &p); CHKERRQ(ierr);
    for (k=pstart; k<pstart+p; k++)
    {
      for (j=nstart; j<nstart+n; j++)
      {
        for (i=mstart; i<mstart+m; i++)
        {
          qx[k][j][i] = flow->initialVelocity[0] * (mesh->dy[j]*mesh->dz[k]);
        }
      }
    }
    ierr = DMDAVecRestoreArray(uda, qxGlobal, &qx); CHKERRQ(ierr);
    // fluxes in y-direction
    PetscReal ***qy;
    ierr = DMDAVecGetArray(vda, qyGlobal, &qy); CHKERRQ(ierr);
    ierr = DMDAGetCorners(vda, &mstart, &nstart, &pstart, &m, &n, &p); CHKERRQ(ierr);
    for (k=pstart; k<pstart+p; k++)
    {
      for (j=nstart; j<nstart+n; j++)
      {
        for (i=mstart; i<mstart+m; i++)
        {
          qy[k][j][i] = flow->initialVelocity[1] * (mesh->dx[i]*mesh->dz[k]);
        }
      }
    }
    ierr = DMDAVecRestoreArray(vda, qyGlobal, &qy); CHKERRQ(ierr);
    // fluxes in z-direction
    PetscReal ***qz;
    ierr = DMDAVecGetArray(wda, qzGlobal, &qz); CHKERRQ(ierr);
    ierr = DMDAGetCorners(wda, &mstart, &nstart, &pstart, &m, &n, &p); CHKERRQ(ierr);
    for (k=pstart; k<pstart+p; k++)
    {
      for (j=nstart; j<nstart+n; j++)
      {
        for (i=mstart; i<mstart+m; i++)
        { 
          qz[k][j][i] = flow->initialVelocity[2] * (mesh->dx[i]*mesh->dy[j]);
        }
      }
    }
    ierr = DMDAVecRestoreArray(wda, qzGlobal, &qz); CHKERRQ(ierr);

    ierr = DMCompositeRestoreAccess(qPack, q, &qxGlobal, &qyGlobal, &qzGlobal); CHKERRQ(ierr);

    if (flow->perturbationAmplitude != 0.0)
    {
      ierr = addInitialPerturbation(); CHKERRQ(ierr);
    }
  }
  
  ierr = DMCompositeScatter(qPack, q, qxLocal, qyLocal, qzLocal); CHKERRQ(ierr);

  return 0;
} // initializeFluxes
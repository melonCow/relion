#include <src/args.h>
#include <mpi.h>
#include <src/ctf.h>
#include <src/jaz/tomography/tomogram.h>
#include <src/jaz/tomography/motion/motion_fit.h>
#include <src/jaz/tomography/motion/proto_alignment.h>
#include <src/jaz/tomography/motion/trajectory_set.h>
#include <src/jaz/tomography/projection_IO.h>
#include <src/jaz/tomography/prediction.h>
#include <src/jaz/tomography/extraction.h>
#include <src/jaz/image/interpolation.h>
#include <src/jaz/util/zio.h>
#include <src/jaz/util/index_sort.h>
#include <src/jaz/optimization/nelder_mead.h>
#include <src/jaz/optimization/gradient_descent.h>
#include <src/jaz/optimization/lbfgs.h>
#include <src/jaz/math/fcc.h>
#include <src/jaz/util/log.h>
#include <src/jaz/tomography/programs/align_mpi.h>

using namespace gravis;

AlignProgramMpi::AlignProgramMpi(int argc, char *argv[])
		: AlignProgram(argc, argv)
{
	// Define a new MpiNode
	node = new MpiNode(argc, argv);
	rank = node->rank;
	nodeCount = node->size;

	// Don't put any output to screen for mpi slaves
	verb = (node->isMaster()) ? 1 : 0;

	if (nodeCount < 2)
	{
		REPORT_ERROR("SubtomoProgramMpi::read: this program needs to be run with at least two MPI processes!");
	}
}

void AlignProgramMpi::run()
{
	if (verb > 0)
	{
		Log::beginSection("Initialising");
	}

	initialise();

	AberrationsCache aberrationsCache(particleSet.optTable, boxSize);

	if (verb > 0)
	{
		Log::endSection();
	}

	std::vector<std::vector<int>> tomoIndices = ParticleSet::splitEvenly(particles, nodeCount);

	processTomograms(tomoIndices[rank], aberrationsCache, verb, false);

	MPI_Barrier(MPI_COMM_WORLD);
	if (node->isMaster())
	{
		finalise();
	}

}

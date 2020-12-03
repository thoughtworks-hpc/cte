#! /bin/sh

if [ -z "${1}" ]; then
  echo "usage: trigger_github_deploy_workflow.sh 'username:personal_access_token'"
  exit 0
fi

curl \
  -u "${1}" \
  -X POST \
  -H "Accept: application/vnd.github.v3+json" \
  https://api.github.com/repos/thoughtworks-hpc/cte/actions/workflows/publish_and_deploy_image.yml/dispatches \
  -d '{"ref": "optimize_write_chunk_data_to_db",
  "inputs": {"DEBUG_FLAG": "0", "NUM_OF_REQUEST": "100000", "SKIP_PUBLISH_DOCKER_IMAGE": "false"}
  }'

# DEBUG_FLAG = 1 ->  order_manager will not write data to db
# SKIP_PUBLISH_DOCKER_IMAGE = false -> workflow will update new docker image to docker hub

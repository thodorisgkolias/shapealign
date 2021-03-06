#### Matching algorithm with C functions ##### Updated : 26/01/2017 ### Check line
#### 187 should be comb2 or comb ###


ProteinAlign <- function(data, SP = 10, n.cores = 8, volume = NULL, jumps = 0,
                         PAM = NULL, gap_open = 0, gap_ext = 0,
                         restrict = FALSE) {
  if (!is.list(data)) {
    stop("data must be a list.")
  } else {
    nam <- lapply(data, names)
    if (any(unlist(lapply(nam, function(x) x != c("amino", "x", "y", "z"))))) {
      stop("data must be in the format of readpdb.")
    }
  }
  if (is.matrix(SP)) {
    if (dim(SP)[1] < 5 | dim(SP)[2] != length(data)) {
      stop("Starting points must be at least 5 landmarks for each molecule")
    }
  } else {
    if (length(SP) != 1 | SP < 5) {
      stop("Starting points must be at least 5 landmarks for each molecule")
    }
  }
  if (restrict) {
    HungarianStep <- HungarianStep1
  } else {
    HungarianStep <- HungarianStep
  }
  if (Sys.info()["sysname"] == "Windows") {
    mclapply <- mclapply.win
    print("NOTE: parLapply is used for parallel computing.")
  }
  start.time <- proc.time()[3]
  
  if (!is.null(PAM)) {
    PAM <- PamTransf(PAM)
  }
  
  DATA <- list()
  DATA[[1]] <- as.matrix(data[[1]][, -1])
  DATA[[2]] <- as.matrix(data[[2]][, -1])
  amino_acid <- list()
  amino_acid[[1]] <- data[[1]][, 1]
  amino_acid[[2]] <- data[[2]][, 1]
  
  cores <- n.cores
  dims <- sapply(DATA, dim)[1, ]
  sm <- order(dims)
  X <- list()
  X <- DATA[sm]
  seq <- list()
  seq <- amino_acid[sm]
  N <- length(X)
  M1 <- dims[sm]
  m <- dim(X[[1]])[2]
  M <- M1
  
  if (is.null(dim(SP))) {
    SPn <- SP
    SP <- SpChoose(data[[1]], data[[2]], k = SPn)
    if (is.null(dim(SP)[1])) {
      SP <- matrix(0, 2, 2)
    }
    if (dim(SP)[1] < SPn) {
      print("Looking again for Starting Points")
      SP <- SpChoose(data[[1]], data[[2]], k = SPn, cut = 0.1)
      if (is.null(dim(SP)[1])) {
        SP <- matrix(0, 2, 2)
      }
      if (dim(SP)[1] < SPn) {
        print("Looking again for Starting Points")
        SP <- SpChoose(data[[1]], data[[2]], k = SPn, cut = 0.05)
      }
    }
  }
  if (dim(SP)[1] < 4) {
    stop("Unable to find good starting point - User defined starting
         point needed")
  }
  
  hist <- SP[, sm]
  p <- dim(hist)[1]
  if (is.null(volume)) {
    vol <- max(sapply(DATA, VolFunc))
  } else {
    vol <- volume
  }
  ## Starting likelihood
  pop <- lapply(M1, seq_len)
  lik0 <- LogLikMatch(pair = NULL, data = X, seq = seq, matched = hist,
                      vol = vol, PAM = PAM, gap_open = gap_open,
                      gap_ext = gap_ext)
  rows2 <- list()
  for (i in 1:N) {
    rows2[[i]] <- which(!pop[[i]] %in% hist[, i])
  }
  ## Explore all available pair of landmarks
  SP <- SP[order(SP[, 1]), ]
  comb <- expand.grid(rows2)
  comb <- as.matrix(comb)
  comb2 <- subset(comb, comb[, 1] < SP[1, 1] & comb[, 2] < SP[1, 2])
  
  for (i in 2:dim(SP)[1]) {
    tt <- subset(comb, comb[, 1] > SP[i - 1, 1] & comb[, 1] < SP[i, 1] &
                 comb[, 2] > SP[i - 1, 2] & comb[, 2] < SP[i, 2])
    comb2 <- rbind(comb2, tt)
  }
  
  tt <- subset(comb, comb[, 1] > SP[i, 1] & comb[, 1] <= M[1] &
               comb[, 2] > SP[i, 2] & comb[, 2] <= M[2])
  comb2 <- rbind(comb2, tt)
  if (restrict) {
    LL <- lapply(seq_len(nrow(comb2)), function(i) comb2[i, ])
  } else {
    LL <- lapply(seq_len(nrow(comb)), function(i) comb[i, ])
  }
  print("Exploring pairs")
  
  if (length(LL) == 1) {
    res <- lapply(LL, LogLikMatch, data = X, seq = seq, matched = hist,
                  vol = vol, PAM = PAM, gap_ext = gap_ext, gap_open = gap_open)
  } else {
    res <- mclapply(LL, LogLikMatch, mc.cores = cores, data = X, seq = seq,
                    matched = hist, vol = vol, PAM = PAM, gap_ext = gap_ext,
                    gap_open = gap_open)
  }
  
  res <- as.numeric(unlist(res))
  ### Apply the Hungarian algorithm to obtain an alignment
  print("Add pairs")
  hung <- HungarianStep(data = X, seq = seq, lik = res, lik0 = lik0,
                        matched = hist, comb2 = comb2, rows2 = rows2,
                        vol = vol, PAM = PAM, gap_open = gap_open, 
                        gap_ext = gap_ext)
  lik0 <- hung$lik0
  hist <- hung$hist
  
  ### REMOVE STEP
  print("Remove pairs")
  res_remove <- RemoveStep(data = X, seq = seq, matched = hist, vol = vol,
                           lik0 = lik0, PAM = PAM, gap_open = gap_open,
                           gap_ext = gap_ext)
  lik0 <- res_remove$lik0
  hist <- res_remove$hist
  pnew <- dim(hist)[1]
  
  # RANDOM JUMP STEP
  if (jumps != 0 & pnew < min(M1)) {
    jump_counter <- 1
    while (jump_counter <= jumps) {
      print(c("Random Jump", jump_counter))
      avail <- list()
      for (i in 1:N) {
        avail[[i]] <- which(!1:M1[i] %in% hist[, i])
      }
      random_pair <- as.vector(unlist(lapply(avail, sample, 1)))
      lik0 <- LogLikMatch(pair = random_pair, data = X, seq = seq,
                          matched = hist, vol = vol, PAM = PAM,
                          gap_open = gap_open, gap_ext = gap_ext)
      hist <- rbind(hist, random_pair)
      rows2 <- list()
      for (i in 1:N) {
        rows2[[i]] <- which(!pop[[i]] %in% hist[, i])
      }
      comb <- expand.grid(rows2)
      comb <- as.matrix(comb)
      LL <- lapply(seq_len(nrow(comb)), function(i) comb[i, ])
      res <- NULL
      if (length(LL) == 1) {
        res <- lapply(LL, LogLikMatch, data = X, seq = seq, matched = hist, 
                      vol = vol, PAM = PAM, gap_ext = gap_ext,
                      gap_open = gap_open)
      } else {
        res <- mclapply(LL, LogLikMatch, mc.cores = cores, data = X, seq = seq, 
                        matched = hist, vol = vol, PAM = PAM, gap_ext = gap_ext,
                        gap_open = gap_open)
      }
      res <- unlist(res)
      res <- as.numeric(res)
      print("Add Pairs")
      hung <- HungarianStep(data = X, seq = seq, lik = res, lik0 = lik0,
                            matched = hist, comb2 = comb, rows2 = rows2,
                            vol = vol, PAM = PAM, gap_open = gap_open, 
                            gap_ext = gap_ext)
      lik0 <- hung$lik0
      hist <- hung$hist
      res_remove <- RemoveStep(data = X, seq = seq, matched = hist, vol = vol, 
                               lik0 = lik0, PAM = PAM, gap_open = gap_open,
                               gap_ext = gap_ext)
      lik0 <- res_remove$lik0
      hist <- res_remove$hist
      pnew <- dim(hist)[1]
      
      jump_counter <- jump_counter + 1
      
      if (pnew == min(M1)) {
        break
      }
      
    }
  } else {
    if (jumps == 0) {
      print("No Random Jumps")
    } else {
      print("No available landmarks for random jumps")
    }
  }
  
  rownames(hist) <- c()
  names(lik0) <- c()
  metric <- SimilarityMetrics(as.matrix(X[[1]]), as.matrix(X[[2]]), hist)
  end.time <- proc.time()[3] - start.time
  names(end.time) <- c("seconds")
  out <- list()
  out$align <- hist
  out$M <- dim(hist)[1]
  out$rmsd <- metric$rmsd
  out$TMscore <- metric$TMscore
  out$SO <- metric$SO
  out$lik <- lik0
  out$time <- end.time
  return(out)
  
}
